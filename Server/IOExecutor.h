#pragma once
#include "Singleton.hpp"
#include "LFQueue.hpp"
#include "SendBuffer.h"

class Session;

struct SendEvent
{
	const uint64_t id;
	class SendBuffer* const send_buff;
};

class IOExecutor
	:public Singleton<IOExecutor>
{
	friend class Singleton;
	IOExecutor() = default;
	~IOExecutor() = default;

	static constexpr int NUM_OF_CLIENTS = 10;
public:
	bool InitServer(std::string_view port);
	void IORoutine()noexcept;
	void EndServer()noexcept { m_bIsRunning = false; }
	const auto& GetIsRunning()const noexcept { return m_bIsRunning; }
public:
	static const auto GetObjectIDAndIncrement()noexcept { return g_GlobalObjectID.fetch_add(1); }
	auto GetSession(const uint64_t id)const noexcept {
		const auto iter = m_mapSession.find(id);
		return m_mapSession.cend() != iter ? iter->second : nullptr;
	}
	template<typename T>
	void AppendToSendBuffer(T&& pkt_)noexcept { m_sendBuff.Append<T>(std::forward<T>(pkt_)); }
	const auto& GetAllSessions()const noexcept { return m_mapSession; }
	void PostWorldSendBuffer(SendBuffer* const pBuff)noexcept { m_broadCastQueue.Push(pBuff); }
	void PostSendQueue(const uint32_t id_, SendBuffer* const pBuff)noexcept {
		m_sendQueue.Push(SendEvent{ id_,pBuff });
	}
private:
	void OnAccept()noexcept;
	void OnDisconnect(const SOCKET sock, const int idx)noexcept;
	void OnRecv(const SOCKET sock)noexcept;
private:
	void FlushSendQueue()noexcept;
private:
	bool m_bIsRunning = false;
	SOCKET m_serverSocket = INVALID_SOCKET;
	WSAPOLLFD m_clientsFD[NUM_OF_CLIENTS + 1];
	int m_curNumOfClient = 0;
	
	std::vector<SendBuffer*> m_flush_buffer;
	LFQueue<SendEvent> m_sendQueue;
	LFQueue<SendBuffer> m_broadCastQueue;

	std::unordered_map<uint64_t, std::shared_ptr<Session>> m_mapSession;
	std::unordered_map<SOCKET, std::shared_ptr<Session>> m_mapSocket2Session;

	SendBuffer m_sendBuff;

	static constinit inline std::atomic<uint32_t> g_GlobalObjectID = 1;
};

static inline const auto IssueGlobalObjectID()noexcept { return IOExecutor::GetInst()->GetObjectIDAndIncrement(); }