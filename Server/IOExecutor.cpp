#include "pch.h"
#include "IOExecutor.h"
#include "Session.h"
#include "PacketBase.hpp"
#include "c2s_PacketHandler.h"

bool IOExecutor::InitServer(std::string_view port)
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        std::cerr << "WSAStartup 실패" << std::endl;
        return false;
    }

    for (int i = 0; i < NUM_OF_CLIENTS + 1; ++i)
    {
        m_clientsFD[i].fd = INVALID_SOCKET;
        m_clientsFD[i].events = 0;
        m_clientsFD[i].revents = 0;
    }

    m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (m_serverSocket == INVALID_SOCKET) 
    {
        std::cerr << "소켓 생성 실패" << std::endl;
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(std::stoi(port.data()));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "바인딩 실패" << std::endl;
        closesocket(m_serverSocket);
        WSACleanup();
        return false;
    }

    if (listen(m_serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "수신 대기 실패" << std::endl;
        closesocket(m_serverSocket);
        WSACleanup();
        return false;
    }

    m_clientsFD[0].fd = m_serverSocket;
    m_clientsFD[0].events = POLLRDNORM;

    std::cout << "서버가 포트 " << port << "에서 실행 중입니다." << std::endl;

    m_bIsRunning = true;

    return true;
}

void IOExecutor::IORoutine() noexcept
{
    while (m_bIsRunning)
    {
        const int pollResult = WSAPoll(m_clientsFD, NUM_OF_CLIENTS + 1, 0);

        if (SOCKET_ERROR == pollResult)
        {
            std::cerr << "WSAPoll 실패: " << WSAGetLastError() << std::endl;
            break;
        }
        
        if (1 <= pollResult)
        {
            // 연결 요청 수락 및 데이터 처리
            if (m_clientsFD[0].revents & POLLRDNORM)
            {
                // TODO: accept + 월드 입장 + 브로드캐스트
                OnAccept();
            }

            for (int i = 1; i <= m_curNumOfClient+1;)
            {
                if (m_clientsFD[i].revents & POLLRDNORM)
                {
                    // TODO: Recv루틴
                    OnRecv(m_clientsFD[i].fd);
                }
                else if (m_clientsFD[i].revents & POLLHUP)
                {
                    // TODO: Disconnect
                    // TODO: 클라이언트 퇴장 브로드캐스팅

                    OnDisconnect(m_clientsFD[i].fd, i);
                    continue;
                }
                ++i;
            }
        }
    }
        // TODO: SendQueue Flush
}

void IOExecutor::OnAccept() noexcept
{
    if (NUM_OF_CLIENTS <= m_curNumOfClient)return;

    const int cur_idx = ++m_curNumOfClient;

    SOCKADDR_STORAGE sockAddr;
    int nAddrLen = sizeof(SOCKADDR_STORAGE);
    m_clientsFD[cur_idx].fd = accept(m_serverSocket, (LPSOCKADDR)&sockAddr, &nAddrLen);
    m_clientsFD[cur_idx].events = POLLRDNORM;

    const auto cur_ID = GetObjectIDAndIncrement();
    const auto session = std::make_shared<Session>(cur_ID, m_clientsFD[cur_idx].fd);
    m_mapSession.try_emplace(cur_ID, session);
    m_mapSocket2Session.try_emplace(m_clientsFD[cur_idx].fd, session);

    std::cout << "Client In\n";
    // TODO: 클라 입장 브로드 캐스팅
}

void IOExecutor::OnDisconnect(const SOCKET sock ,const int idx) noexcept
{
    const auto iter = m_mapSocket2Session.find(sock);
    if (m_mapSocket2Session.cend() == iter)return;
    const auto& session = iter->second;

    const auto session_id = session->GetSessionID();

    const auto last_idx = m_curNumOfClient--;
    std::swap(m_clientsFD[idx], m_clientsFD[last_idx]);

    m_clientsFD[last_idx].fd = INVALID_SOCKET;
    m_clientsFD[last_idx].events = 0;
    m_clientsFD[last_idx].revents = 0;

    m_mapSocket2Session.erase(iter);
    m_mapSession.erase(session_id);

    std::cout << "Client Out\n";
}

void IOExecutor::OnRecv(const SOCKET sock) noexcept
{
    const auto iter = m_mapSocket2Session.find(sock);
    if (m_mapSocket2Session.cend() == iter)return;
    const auto& session = iter->second;

    const auto recv_buff = session->GetRecvBuffer();

    const int data_len = recv(sock, recv_buff->WritePos(), RecvBuffer::RECV_BUFFER_SIZE::BUFFER_SIZE, 0);
    recv_buff->OnWrite(data_len);

    const int process_len = ::OnRecv(session->GetSessionID(),recv_buff->ReadPos(), recv_buff->DataSize(), c2s_PacketHandler::GetPacketHandlerList());

    recv_buff->OnRead(process_len);

    recv_buff->Clear();
}
