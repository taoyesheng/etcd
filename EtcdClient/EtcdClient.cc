//2018-10-15
//tys create
// etcd client realization

#include <cstdio>
#include<string.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <grpcpp/grpcpp.h>

#include "EtcdClient.h"
#include "rpc.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace etcdclient
{
	class  EtcdClient::EtcdClientImpl
	{
	public:
		class KVImpl final
		{
		public:
			KVImpl(const string& sIpAndPort);
			~KVImpl();

			const string& GetsIpAndPort() const;

			int Range(const etcdserverpb::RangeRequest& request, etcdserverpb::RangeResponse& response);
			int Put(const etcdserverpb::PutRequest& request, etcdserverpb::PutResponse& response);
			int DeleteRange(const etcdserverpb::DeleteRangeRequest& request, etcdserverpb::DeleteRangeResponse& response);
			int Txn(const etcdserverpb::TxnRequest& request, etcdserverpb::TxnResponse& response);
			int Compact(const etcdserverpb::CompactionRequest& request, etcdserverpb::CompactionResponse& response);

		private:
			unique_ptr<::etcdserverpb::KV::Stub>    m_stubKV;
			string m_sIpAndPort;
		};

		class WatchImpl final
		{
		public:
			WatchImpl(const string& sIpAndPort);
			~WatchImpl();

			int  Watch();
			void TakeResponse(std::shared_ptr<WatchResponse>& sPtrResponse);
			void TakResponses(std::map<int64,vector<std::shared_ptr<WatchResponse> > >& outRespons);
			void TakResponses(std::list<std::shared_ptr<WatchResponse> >& outRespons);
			bool ResponsesEmpty();
			void SetpWatchCallBackBase(WatchCallBackBase* pInput);
			int  Write(const etcdserverpb::WatchRequest& request);

			void RunInThread();

			inline const string& GetsIpAndPort() const { return m_sIpAndPort; }
			inline void          SetbStore(bool bInput = false) { m_bStoreRes.store(bInput); }
			inline const bool    GetbStore() const { return m_bStoreRes; }
			inline const bool    GetbWatchNormal()const { return m_bWatchNormal.load(); }
			void                 SetnIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen);
			
		private:
			
			void CopyWatchResponse(const etcdserverpb::WatchResponse& Input, WatchResponse& Output);
			void Stop();
			
			bool               m_bInitWatch       = false    ;
			std::mutex         m_mutexData                   ;
			std::atomic_bool   m_bRuning                     ;
			std::atomic_bool   m_bStoreRes                   ;
			std::atomic_bool   m_bWatchNormal                ;
			std::atomic_int    m_nInterval                   ;
			std::atomic_int    m_nMaxDataLen                 ;
			WatchCallBackBase* m_pWatchCallBackBase=nullptr  ;
			std::string m_sIpAndPort                         ;
		
			std::shared_ptr<ClientContext> m_sPtrClientContext;
			std::unique_ptr< ::grpc::ClientReaderWriter< ::etcdserverpb::WatchRequest, ::etcdserverpb::WatchResponse> > m_uPtrClientRW;
			std::unique_ptr<::etcdserverpb::Watch::Stub> m_stubWatch;
			std::multimap<chrono::time_point<chrono::high_resolution_clock>, std::shared_ptr<WatchResponse> > m_multiMapResPonse;
			std::shared_ptr<thread>    m_thread;
		};

		class LeaseImpl final
		{
		public:

			LeaseImpl(const string& sIpAndPort);
			~LeaseImpl();

			int   LeaseGrant(const ::etcdserverpb::LeaseGrantRequest& request, ::etcdserverpb::LeaseGrantResponse& response);
			int   LeaseRevoke(const ::etcdserverpb::LeaseRevokeRequest& request, ::etcdserverpb::LeaseRevokeResponse& response);
			int   LeaseKeepAlive();
			void  SetpLeaseCallBackBase(LeaseCallBackBase* pInput = nullptr);
			int   LeaseKeepAliveWrite(const etcdserverpb::LeaseKeepAliveRequest& request);
			int   LeaseTimeToLive(const ::etcdserverpb::LeaseTimeToLiveRequest& request, etcdserverpb::LeaseTimeToLiveResponse& response);

			void  TakeResponse(std::shared_ptr<LeaseKeepAliveResponse>& sPtrResponse);
			void  TakResponses(std::list<std::shared_ptr<LeaseKeepAliveResponse> >& outRespons);
			bool  ResponsesEmpty();

			void  RunInThread();

			const string& GetsIpAndPort() const { return m_sIpAndPort; }

			inline const bool GetbLeaseNormal()const { return m_bLeaseNormal.load(); }
			inline void       SetbStoreRes(bool bStore)   { m_bStoreRes.exchange(bStore); }
			void              SetnIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen);
						      
		private:

			void CopyLeaseKeepAliveResponse(const etcdserverpb::LeaseKeepAliveResponse& Input, LeaseKeepAliveResponse& Output);
			void Stop();

			std::mutex                 m_mutexData                       ;
			std::atomic_bool           m_bStoreRes                       ;
			std::atomic_bool           m_bInitKeepAlive                  ;
			std::atomic_bool           m_bRuning                         ;
			std::atomic_bool           m_bLeaseNormal                    ;
			std::atomic_int            m_nInterval                       ;
			std::atomic_int            m_nMaxDataLen                     ;
			LeaseCallBackBase*         m_pLeaseCallBackBase  = nullptr   ;
		
			std::string m_sIpAndPort;
			std::shared_ptr<ClientContext>      m_ptrClientContext;
			std::unique_ptr<grpc::ClientReaderWriter<etcdserverpb::LeaseKeepAliveRequest, etcdserverpb::LeaseKeepAliveResponse> > m_uPtrClientRW;
			std::unique_ptr<::etcdserverpb::Lease::Stub> m_stubLease;
			std::multimap<chrono::time_point<chrono::high_resolution_clock>, std::shared_ptr<LeaseKeepAliveResponse> > m_multiMapResPonse;
			
			std::shared_ptr<thread>    m_thread;
		};

		class AuthImpl final
		{
		public:
			AuthImpl(const string& sIpAndPort);
			~AuthImpl();
			
			int AuthEnable(const ::etcdserverpb::AuthEnableRequest& request,::etcdserverpb::AuthEnableResponse& response);
			int AuthDisable(const ::etcdserverpb::AuthDisableRequest& request, ::etcdserverpb::AuthDisableResponse& response);
			int Authenticate(const ::etcdserverpb::AuthenticateRequest& request, ::etcdserverpb::AuthenticateResponse& response);
			int UserAdd(const ::etcdserverpb::AuthUserAddRequest& request, ::etcdserverpb::AuthUserAddResponse& response); 
			int UserGet(const ::etcdserverpb::AuthUserGetRequest& request, ::etcdserverpb::AuthUserGetResponse& response); 
			int UserList(const ::etcdserverpb::AuthUserListRequest& request, ::etcdserverpb::AuthUserListResponse& response); 
			int UserDelete(const ::etcdserverpb::AuthUserDeleteRequest& request, ::etcdserverpb::AuthUserDeleteResponse& response); 
			int UserChangePassword(const ::etcdserverpb::AuthUserChangePasswordRequest& request, ::etcdserverpb::AuthUserChangePasswordResponse& response);
			int UserGrantRole(const ::etcdserverpb::AuthUserGrantRoleRequest& request, ::etcdserverpb::AuthUserGrantRoleResponse& response); 
			int UserRevokeRole(const ::etcdserverpb::AuthUserRevokeRoleRequest& request, ::etcdserverpb::AuthUserRevokeRoleResponse& response); 
			int RoleAdd(const ::etcdserverpb::AuthRoleAddRequest& request, ::etcdserverpb::AuthRoleAddResponse& response); 
			int RoleGet(const ::etcdserverpb::AuthRoleGetRequest& request, ::etcdserverpb::AuthRoleGetResponse& response); 
			int RoleList(const ::etcdserverpb::AuthRoleListRequest& request, ::etcdserverpb::AuthRoleListResponse& response); 
			int RoleDelete(const ::etcdserverpb::AuthRoleDeleteRequest& request, ::etcdserverpb::AuthRoleDeleteResponse& response); 
			int RoleGrantPermission(const ::etcdserverpb::AuthRoleGrantPermissionRequest& request, ::etcdserverpb::AuthRoleGrantPermissionResponse& response); 
			int RoleRevokePermission(const ::etcdserverpb::AuthRoleRevokePermissionRequest& request, ::etcdserverpb::AuthRoleRevokePermissionResponse& response); 
		private:
			unique_ptr<::etcdserverpb::Auth::Stub>    m_stubAuth;
			string m_sIpAndPort;
		};

		void InitKVImpl(const string& sIpAndPort);
		void InitWatch(const string& sIpAndPor);
		void InitLease(const string& sIpAndPor);
		void InitAuth(const string& sIpAndPor);

	    string GetKVIpAndPort() const;
		string GetWatchIpAndProt() const;
		
		inline shared_ptr<WatchImpl> GetsPtrWathImpl() { return m_sPtrWathImpl; }

		int Range(const RangeRequest& request, RangeResponse& response);
		int Put(const PutRequest& request, PutResponse& response);
		int DeleteRange(const DeleteRangeRequest& request, DeleteRangeResponse& response);
		int Txn(const TxnRequest& request, TxnResponse& response);
		int Compact(const CompactionRequest& request, CompactionResponse& response);

		int Watch(WatchCallBackBase* pClassCallBack = nullptr);
		int Write(const WatchRequest& request);
		const bool    GetbWatchNormal()const;
		void          SetnIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen);


		int LeaseGrant(const LeaseGrantRequest& request, LeaseGrantResponse& response);
		int LeaseRevoke(const LeaseRevokeRequest& request, LeaseRevokeResponse& response);
		int LeaseTimeToLive(const LeaseTimeToLiveRequest& request, LeaseTimeToLiveResponse& response);

		int LeaseKeepAlive(LeaseCallBackBase* pInput = nullptr);
		int LeaseKeepAliveWrite(const LeaseKeepAliveRequest& request);
		const bool GetbLeaseNormal()const;
		void  TakeResponse(std::shared_ptr<LeaseKeepAliveResponse>& sPtrResponse);
		void  TakResponses(std::list<std::shared_ptr<LeaseKeepAliveResponse> >& outRespons);
		void  SetLeasenIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen);
		void  SetLeasebStoreRes(bool bStore);
		bool  LeaseResponsesEmpty();


		int AuthEnable(const AuthEnableRequest& request, AuthEnableResponse& response);
		int AuthDisable(const AuthDisableRequest& request, AuthDisableResponse& response);
		int Authenticate(const AuthenticateRequest& request, AuthenticateResponse& response);
		int UserAdd(const AuthUserAddRequest& request, AuthUserAddResponse& response);
		int UserGet(const AuthUserGetRequest& request, AuthUserGetResponse& response);
		int UserList(const AuthUserListRequest& request, AuthUserListResponse& response);
		int UserDelete(const AuthUserDeleteRequest& request,AuthUserDeleteResponse& response);
		int UserChangePassword(const AuthUserChangePasswordRequest& request, AuthUserChangePasswordResponse& response);
		int UserGrantRole(const AuthUserGrantRoleRequest& request, AuthUserGrantRoleResponse& response);
		int UserRevokeRole(const AuthUserRevokeRoleRequest& request, AuthUserRevokeRoleResponse& response);
		int RoleAdd(const AuthRoleAddRequest& request, AuthRoleAddResponse& response);
		int RoleGet(const AuthRoleGetRequest& request, AuthRoleGetResponse& response);
		int RoleList(const AuthRoleListRequest& request,AuthRoleListResponse& response);
		int RoleDelete(const AuthRoleDeleteRequest& request, AuthRoleDeleteResponse& response);
		int RoleGrantPermission(const AuthRoleGrantPermissionRequest& request, AuthRoleGrantPermissionResponse& response);
		int RoleRevokePermission(const AuthRoleRevokePermissionRequest& request, AuthRoleRevokePermissionResponse& response);
	

		static 	string RetAddOneKey(const string& sInput);

		static void CopyHeader(const ::etcdserverpb::ResponseHeader& input, DefType<ResponseHeader>& output);
		static void CopyEvent(const ::mvccpb::Event& input, Event& output);
		static void CopyKeyValue(const ::mvccpb::KeyValue& input, KeyValue& output);
		static void CopyKeyValue(const ::mvccpb::KeyValue& input, DefType<KeyValue>& output);
		static void CopyEventVector(const ::google::protobuf::RepeatedPtrField< ::mvccpb::Event >& input, vector<Event >& output);

	private:

		void CopyKeyValueVector(const ::google::protobuf::RepeatedPtrField< ::mvccpb::KeyValue >& input, vector<KeyValue >& output);
		int  CopyRangeRequest(const RangeRequest &request, ::etcdserverpb::RangeRequest &requestpb);
		void CopyRangResponse(const ::etcdserverpb::RangeResponse &responsepb, RangeResponse &response);
		void CopyPutRequest(const PutRequest &request, ::etcdserverpb::PutRequest &reqgrpc);
		void CopyPutResponse(const ::etcdserverpb::PutResponse &resgrpc, PutResponse &response);
		int  CopyDeleteRangeRequest(const DeleteRangeRequest &request, etcdserverpb::DeleteRangeRequest &reqgrpc);
		void CopyDeleteRangeResponse(const etcdserverpb::DeleteRangeResponse &resgrpc, DeleteRangeResponse &response);
		void CopyTxnRequest(const TxnRequest &request, etcdserverpb::TxnRequest &reqgrcp);
		void CopyTxnResponse(const etcdserverpb::TxnResponse& resgrpc, TxnResponse& response);
		void CopyRequestOpVector(const std::vector<RequestOp>& Input, ::google::protobuf::RepeatedPtrField< ::etcdserverpb::RequestOp >*Output);
		void CopyResponseOpVector(const ::google::protobuf::RepeatedPtrField< ::etcdserverpb::ResponseOp >& Input, std::vector<ResponseOp>& Output);


		std::unique_ptr<KVImpl>     m_uPtrKVImpl    ;
		std::shared_ptr<WatchImpl>  m_sPtrWathImpl  ;
		std::unique_ptr<LeaseImpl>  m_uPtrLeaseImpl ;
		std::unique_ptr<AuthImpl>   m_uPtrAuthImpl  ;
	};

	EtcdClient::EtcdClientImpl::LeaseImpl::LeaseImpl(const string& sIpAndPort)
	{
		m_sIpAndPort = sIpAndPort;
		m_stubLease  = ::etcdserverpb::Lease::NewStub(grpc::CreateChannel(sIpAndPort, grpc::InsecureChannelCredentials()));
		m_bLeaseNormal.store(true);
		m_bStoreRes.store(false);
		m_nInterval.store(3);
		m_nMaxDataLen.store(2000);
	}

	EtcdClient::EtcdClientImpl::LeaseImpl::~LeaseImpl()
	{
		Stop();
	}

	int EtcdClient::EtcdClientImpl::LeaseImpl::LeaseGrant(const ::etcdserverpb::LeaseGrantRequest& request, ::etcdserverpb::LeaseGrantResponse& response)
	{
		ClientContext context;
		Status status = m_stubLease->LeaseGrant(&context, request, &response);
		if (status.ok())
		{
#if _PRINT_RES_STRING_
			std::string sRe = response.DebugString();
			response.PrintDebugString();
#endif
			return E_G_VALUE::SUCCESS;
		}
		else
		{
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;

			return E_G_VALUE::RPC_RET_ERR;
		}
	}

	int EtcdClient::EtcdClientImpl::LeaseImpl::LeaseRevoke(const ::etcdserverpb::LeaseRevokeRequest& request, ::etcdserverpb::LeaseRevokeResponse& response)
	{
		ClientContext context;
		Status status = m_stubLease->LeaseRevoke(&context, request, &response);
		if (status.ok())
		{
#if _PRINT_RES_STRING_
			std::string sRe = response.DebugString();
			response.PrintDebugString();
#endif
			return E_G_VALUE::SUCCESS;
		}
		else
		{
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;

			return E_G_VALUE::RPC_RET_ERR;
		}
	}

	int EtcdClient::EtcdClientImpl::LeaseImpl::LeaseTimeToLive(const ::etcdserverpb::LeaseTimeToLiveRequest& request, etcdserverpb::LeaseTimeToLiveResponse& response)
	{
		ClientContext context;
		Status status = m_stubLease->LeaseTimeToLive(&context, request, &response);
		if (status.ok())
		{
#if _PRINT_RES_STRING_
			std::string sRe = response.DebugString();
			response.PrintDebugString();
#endif
			return E_G_VALUE::SUCCESS;
		}
		else
		{
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;

			return E_G_VALUE::RPC_RET_ERR;
		}
	}

	void EtcdClient::EtcdClientImpl::LeaseImpl::TakeResponse(std::shared_ptr<LeaseKeepAliveResponse>& sPtrResponse)
	{
		if (ResponsesEmpty())
			return;

		sPtrResponse.reset();

		std::unique_lock<mutex> locker(m_mutexData);
		sPtrResponse.swap(m_multiMapResPonse.begin()->second);
		m_multiMapResPonse.erase(m_multiMapResPonse.begin());
	}

	void EtcdClient::EtcdClientImpl::LeaseImpl::TakResponses(std::list<std::shared_ptr<LeaseKeepAliveResponse>>& outRespons)
	{
		outRespons.clear();

		std::multimap<chrono::time_point<chrono::high_resolution_clock>, std::shared_ptr<LeaseKeepAliveResponse> > multiMapRes;

		{
			std::unique_lock<mutex> locker(m_mutexData);
			multiMapRes.swap(m_multiMapResPonse);
		}

		for (const auto& ele : multiMapRes)
		{
			outRespons.push_back(std::move(ele.second));
		}
	}

	int EtcdClient::EtcdClientImpl::LeaseImpl::LeaseKeepAlive()
	{
		if (m_bInitKeepAlive)
			return E_G_VALUE::LEASE_HAVED_INIT;

		m_ptrClientContext = std::make_shared<ClientContext>();
		m_uPtrClientRW     = m_stubLease->LeaseKeepAlive(m_ptrClientContext.get());

		if (nullptr == m_uPtrClientRW)
		{
			return E_G_VALUE::LEASE_FAIL;
		}

		m_bInitKeepAlive.store(true);

		m_bRuning.exchange(true);
		m_thread = std::make_shared<thread>(&EtcdClient::EtcdClientImpl::LeaseImpl::RunInThread,this);
		
		return E_G_VALUE::SUCCESS;
	}

	bool EtcdClient::EtcdClientImpl::LeaseImpl::ResponsesEmpty()
	{
		unique_lock<mutex> locker(m_mutexData);
		return m_multiMapResPonse.empty();
	}

	void EtcdClient::EtcdClientImpl::LeaseImpl::RunInThread()
	{
		::etcdserverpb::LeaseKeepAliveResponse  resgrpc;

		while (m_bRuning && nullptr != m_uPtrClientRW)
		{
			resgrpc.Clear();

			if (m_uPtrClientRW->Read(&resgrpc))
			{

#if _PRINT_RES_STRING_
				std::string sRe = resgrpc.DebugString();
				resgrpc.PrintDebugString();
#endif

				shared_ptr<LeaseKeepAliveResponse> sPtrRes = make_shared<LeaseKeepAliveResponse>();

				CopyLeaseKeepAliveResponse(resgrpc, *sPtrRes);

				if (m_bStoreRes)
				{
					unique_lock<mutex> locker(m_mutexData);
					m_multiMapResPonse.insert(make_pair(chrono::high_resolution_clock::now(), sPtrRes));
				}

				LeaseCallBackBase* pCallBack = nullptr;
				{
					unique_lock<mutex> locker(m_mutexData);
					pCallBack = m_pLeaseCallBackBase;
				}

				if (nullptr != pCallBack)
				{
					pCallBack->RunAfterReadInLease(*sPtrRes);
				}
			}
			else
			{
				m_bLeaseNormal.store(false);
			}

			while (m_bStoreRes)
			{
				shared_ptr<LeaseKeepAliveResponse> sPtrResErase;

				unique_lock<mutex> locker(m_mutexData);
				if (!m_multiMapResPonse.empty())
				{
					std::multimap<chrono::time_point<chrono::high_resolution_clock>, shared_ptr<LeaseKeepAliveResponse>  >::iterator  iter = m_multiMapResPonse.begin();

					int64 nDiff = chrono::duration_cast<chrono::minutes>(chrono::high_resolution_clock::now() - iter->first).count();
					if ((int)nDiff >= m_nInterval.load() || (int)m_multiMapResPonse.size() >= m_nMaxDataLen.load())
					{

						sPtrResErase.swap(iter->second);
						m_multiMapResPonse.erase(iter);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}

			}

			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	void EtcdClient::EtcdClientImpl::LeaseImpl::SetpLeaseCallBackBase(LeaseCallBackBase* pInput /*= nullptr*/)
	{
		unique_lock<mutex> locker(m_mutexData);
		m_pLeaseCallBackBase = pInput;
	}

	void EtcdClient::EtcdClientImpl::LeaseImpl::SetnIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen)
	{
		m_nInterval.exchange(nInterval);
		m_nMaxDataLen.exchange(nMaxDataLen);
	}

	void EtcdClient::EtcdClientImpl::LeaseImpl::CopyLeaseKeepAliveResponse(const etcdserverpb::LeaseKeepAliveResponse& Input, LeaseKeepAliveResponse& Output)
	{
		if (Input.has_header())
			EtcdClient::EtcdClientImpl::CopyHeader(Input.header(), Output.header);

		Output.nID = Input.id();
		Output.nTTL = Input.ttl();
	}

	void EtcdClient::EtcdClientImpl::LeaseImpl::Stop()
	{
		m_bRuning.exchange(false);

		if (nullptr != m_uPtrClientRW)
			m_uPtrClientRW->WritesDone();

		if (nullptr != m_thread && m_thread->joinable())
			m_thread->join();
	}

	int EtcdClient::EtcdClientImpl::LeaseImpl::LeaseKeepAliveWrite(const etcdserverpb::LeaseKeepAliveRequest& request)
	{
		if (nullptr == m_uPtrClientRW)
			return E_G_VALUE::LEASE_NO_LEASING;

		if (m_uPtrClientRW->Write(request))
			return E_G_VALUE::LEASE_WRITE_ERR;

		m_bLeaseNormal.store(false);

		return E_G_VALUE::SUCCESS;
	}

	void EtcdClient::EtcdClientImpl::InitKVImpl(const string& sIpAndPort)
	{
		//m_uPtrKVImpl = std::unique_ptr<KVImpl>(new KVImpl(sIpAndPort));
		std::unique_ptr<KVImpl> uPtrKVImpl = std::unique_ptr<KVImpl>(new KVImpl(sIpAndPort));
		m_uPtrKVImpl.swap(uPtrKVImpl);
	}

	void EtcdClient::EtcdClientImpl::InitWatch(const string& sIpAndPor)
	{
		m_sPtrWathImpl = std::shared_ptr<WatchImpl>(new WatchImpl(sIpAndPor));
	}

	string EtcdClient::EtcdClientImpl::GetKVIpAndPort() const
{
		if(nullptr != m_uPtrKVImpl)
			return m_uPtrKVImpl->GetsIpAndPort();

		return "";
	}

	string EtcdClient::EtcdClientImpl::GetWatchIpAndProt() const
{
		if(nullptr != m_sPtrWathImpl)
			return  m_sPtrWathImpl->GetsIpAndPort();

		return "";
	}

	int EtcdClient::EtcdClientImpl::Range(const RangeRequest& request, RangeResponse& response)
	{
		if (nullptr == m_uPtrKVImpl)
			return E_G_VALUE::KV_NO_INIT;

		::etcdserverpb::RangeRequest  requestpb;
		::etcdserverpb::RangeResponse responsepb;
		
		int nRet = CopyRangeRequest(request, requestpb);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

	    nRet = m_uPtrKVImpl->Range(requestpb, responsepb);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		CopyRangResponse(responsepb, response);

		return E_G_VALUE::SUCCESS;
	}

	string EtcdClient::EtcdClientImpl::RetAddOneKey(const string& sInput)
	{
		string sKey(sInput);

		int nLens = sKey.length();

		unsigned char* pHead = (unsigned char*)sKey.c_str();

		for (int i = 0; i < nLens; i++)
		{
			pHead[nLens - 1 - i] += 1;
			if (0 != pHead[nLens - 1 - i])
			{
				return sKey;
			}
		}
		char cStr = 1;
		return string(&cStr, 1) + sKey;
	}

	void EtcdClient::EtcdClientImpl::CopyHeader(const ::etcdserverpb::ResponseHeader& input, DefType<ResponseHeader>& output)
	{
		output.bHas = true;
		output.value.nClusterId = input.cluster_id() ;
		output.value.nMemberId  = input.member_id()  ;
		output.value.nRevision  = input.revision()   ;
		output.value.nRaftTem   = input.raft_term()  ;
	}

	void EtcdClient::EtcdClientImpl::CopyKeyValueVector(const ::google::protobuf::RepeatedPtrField< ::mvccpb::KeyValue >& input, vector<KeyValue >& output)
	{
		for (int i=0; i< input.size();i++)
		{
			KeyValue outKV;
			CopyKeyValue(input[i], outKV);
			output.push_back(std::move(outKV));
		}
	}

	void EtcdClient::EtcdClientImpl::CopyKeyValue(const ::mvccpb::KeyValue& input, KeyValue& output)
	{
		output.sKey            = input.key()             ;
		output.nCreateRevision = input.create_revision() ;
		output.nModRevision    = input.mod_revision()    ;
		output.nVersion        = input.version()         ;
		output.sValue          = input.value()           ;
		output.nLease          = input.lease()           ;
	}

	void EtcdClient::EtcdClientImpl::CopyKeyValue(const ::mvccpb::KeyValue& input, DefType<KeyValue>& output)
	{
		output.bHas = true;
		EtcdClient::EtcdClientImpl::CopyKeyValue(input, output.value);
	}

	int EtcdClient::EtcdClientImpl::Put(const PutRequest& request, PutResponse& response)
	{
		if (nullptr == m_uPtrKVImpl)
			return E_G_VALUE::KV_NO_INIT;

		::etcdserverpb::PutRequest reqgrpc;
		::etcdserverpb::PutResponse resgrpc;

		CopyPutRequest(request, reqgrpc);

		int nRet = m_uPtrKVImpl->Put(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
			return nRet;

		CopyPutResponse(resgrpc, response);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::DeleteRange(const DeleteRangeRequest& request, DeleteRangeResponse& response)
	{
		if (nullptr == m_uPtrKVImpl)
			return E_G_VALUE::KV_NO_INIT;

		etcdserverpb::DeleteRangeRequest reqgrpc;
		etcdserverpb::DeleteRangeResponse resgrpc;

		int nRet = CopyDeleteRangeRequest(request, reqgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		nRet = m_uPtrKVImpl->DeleteRange(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		CopyDeleteRangeResponse(resgrpc, response);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::Txn(const TxnRequest& request, TxnResponse& response)
	{
		if (nullptr == m_uPtrKVImpl)
			return E_G_VALUE::KV_NO_INIT;

		etcdserverpb::TxnRequest  reqgrcp;
		etcdserverpb::TxnResponse resgrcp;

		CopyTxnRequest(request, reqgrcp);

		int nRet = m_uPtrKVImpl->Txn(reqgrcp, resgrcp);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		CopyTxnResponse(resgrcp,response);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::CopyRangeRequest(const RangeRequest &request, ::etcdserverpb::RangeRequest &requestpb)
	{
		if (request.bAllKey)
		{
			requestpb.set_key("", 1);
			requestpb.set_range_end("", 1);
		}
		else
		{
			if (request.sKey.Empty())
				return E_G_VALUE::INPUT_KEY_ERR;

			requestpb.set_key(request.sKey.GetString());

			if (request.bEqOrGre)
			{
				requestpb.set_range_end("", 1);
			}
			else if (request.bPrefixed)
			{
				requestpb.set_range_end(EtcdClientImpl::RetAddOneKey(request.sKey.GetString()));
			}
			else
			{
				if (!request.sRangeEnd.Empty())
					requestpb.set_range_end(request.sRangeEnd.GetString());
			}
		}


		if (!request.nLimit.Empty())
			requestpb.set_limit(request.nLimit.GetInt());

		if (!request.nRevision.Empty())
			requestpb.set_revision(request.nRevision.GetInt());

		if (!request.eSortOrder.Empty())
			requestpb.set_sort_order((::etcdserverpb::RangeRequest_SortOrder)request.eSortOrder.GetInt());

		if (!request.eSortTarget.Empty())
			requestpb.set_sort_target((::etcdserverpb::RangeRequest_SortTarget)request.eSortTarget.GetInt());

		if (!request.bSerializeble.Empty())
			requestpb.set_serializable(request.bSerializeble.GetBool());

		if (!request.bKeyOnly.Empty())
			requestpb.set_keys_only(request.bKeyOnly.GetBool());

		if (!request.bCountOnly.Empty())
			requestpb.set_count_only(request.bCountOnly.GetBool());

		if (!request.nMinModRevision.Empty())
			requestpb.set_min_mod_revision(request.nMinModRevision.GetInt());

		if (!request.nMaxModRevision.Empty())
			requestpb.set_max_mod_revision(request.nMaxModRevision.GetInt());

		if (!request.nMinCreateRevision.Empty())
			requestpb.set_min_create_revision(request.nMinCreateRevision.GetInt());

		if (!request.nMaxCreateReviosn.Empty())
			requestpb.set_max_create_revision(request.nMaxCreateReviosn.GetInt());

		return E_G_VALUE::SUCCESS;
	}

	void EtcdClient::EtcdClientImpl::CopyRangResponse(const ::etcdserverpb::RangeResponse &responsepb, RangeResponse &response)
	{
		if (responsepb.has_header())
			CopyHeader(responsepb.header(), response.header);

		CopyKeyValueVector(responsepb.kvs(), response.vKvs);

		response.bMore = responsepb.more();
		response.nCount = responsepb.count();
	}

	void EtcdClient::EtcdClientImpl::CopyPutRequest(const PutRequest &request, ::etcdserverpb::PutRequest &reqgrpc)
	{
		if (!request.sKey.Empty())
			reqgrpc.set_key(request.sKey.GetString());

		if (!request.sValue.Empty())
			reqgrpc.set_value(request.sValue.GetString());

		if (!request.nLease.Empty())
			reqgrpc.set_lease(request.nLease.GetInt());

		if (!request.bPreKv.Empty())
			reqgrpc.set_prev_kv(request.bPreKv.GetBool());

		if (!request.bIgnoreValue.Empty())
			reqgrpc.set_ignore_value(request.bIgnoreValue.GetBool());

		if (!request.bIgnoreLease.Empty())
			reqgrpc.set_ignore_lease(request.bIgnoreLease.GetBool());
	}

	void EtcdClient::EtcdClientImpl::CopyPutResponse(const ::etcdserverpb::PutResponse &resgrpc, PutResponse &response)
	{
		if (resgrpc.has_header())
			CopyHeader(resgrpc.header(), response.header);

		if (resgrpc.has_prev_kv())
		{
			CopyKeyValue(resgrpc.prev_kv(), response.preKv);
		}
	}

	int EtcdClient::EtcdClientImpl::CopyDeleteRangeRequest(const DeleteRangeRequest &request, etcdserverpb::DeleteRangeRequest &reqgrpc)
	{

		if (request.bAllKey)
		{
			reqgrpc.set_key("", 1);
			reqgrpc.set_range_end("", 1);
		}
		else
		{
			if (request.sKey.Empty())
				return E_G_VALUE::INPUT_KEY_ERR;

			reqgrpc.set_key(request.sKey.GetString());

			if (request.bEqualOrGreater)
			{
				reqgrpc.set_range_end("", 1);
			}
			else if (request.bPrefixe)
			{
				reqgrpc.set_range_end(EtcdClientImpl::RetAddOneKey(request.sKey.GetString()));
			}
			else
			{
				if (!request.sRangeEnd.Empty())
					reqgrpc.set_range_end(request.sRangeEnd.GetString());
			}
		}

		if (!request.bPrevKv.Empty())
			reqgrpc.set_prev_kv(request.bPrevKv.GetBool());

		return E_G_VALUE::SUCCESS;
	}

	void EtcdClient::EtcdClientImpl::CopyDeleteRangeResponse(const etcdserverpb::DeleteRangeResponse &resgrpc, DeleteRangeResponse &response)
	{
		if (resgrpc.has_header())
			CopyHeader(resgrpc.header(), response.header);

		response.nDeleted = resgrpc.deleted();
		CopyKeyValueVector(resgrpc.prev_kvs(), response.vPreKvs);
	}

	void EtcdClient::EtcdClientImpl::CopyTxnRequest(const TxnRequest &request, etcdserverpb::TxnRequest &reqgrcp)
	{
		for (const auto& vEle : request.vCompare)
		{
			auto pComp = reqgrcp.add_compare();

			if (!vEle.nResult.Empty())
				pComp->set_result((::etcdserverpb::Compare_CompareResult)vEle.nResult.GetInt());

			if (!vEle.nTarget.Empty())
				pComp->set_target((::etcdserverpb::Compare_CompareTarget)vEle.nTarget.GetInt());

			if (!vEle.sKey.Empty())
				pComp->set_key(vEle.sKey.GetString());

			int64 nOut = 0;
			string sValue;

			if (vEle.targetUnion.GetnVersion(nOut))
				pComp->set_version(nOut);

			if (vEle.targetUnion.GetnCreateRevision(nOut))
				pComp->set_create_revision(nOut);

			if (vEle.targetUnion.GetnModRevision(nOut))
				pComp->set_mod_revision(nOut);

			if (vEle.targetUnion.GetsValue(sValue))
				pComp->set_value(std::move(sValue));

			if (!vEle.sRangeEnd.Empty())
				pComp->set_value(std::move(vEle.sRangeEnd.GetString()));

		}

		CopyRequestOpVector(request.vSuccess, reqgrcp.mutable_success());
		CopyRequestOpVector(request.vFailure,reqgrcp.mutable_failure());
	}

	void EtcdClient::EtcdClientImpl::CopyRequestOpVector(const std::vector<RequestOp>& Input, ::google::protobuf::RepeatedPtrField< ::etcdserverpb::RequestOp >*Output)
	{
		for (const auto& vEle : Input)
		{
			auto pReqOp = Output->Add();

			if (nullptr != vEle.GetConstsPtrRequestRang())
				CopyRangeRequest(*vEle.GetConstsPtrRequestRang(), *pReqOp->mutable_request_range());

			if (nullptr != vEle.GetConstsPtrRequestPut())
				CopyPutRequest(*vEle.GetConstsPtrRequestPut(), *pReqOp->mutable_request_put());

			if (nullptr != vEle.GetConstsPtrRequestDeleteRangRequest())
				CopyDeleteRangeRequest(*vEle.GetConstsPtrRequestDeleteRangRequest(), *pReqOp->mutable_request_delete_range());

			if (nullptr != vEle.GetConstsPtrRequestTxn())
				CopyTxnRequest(*vEle.GetConstsPtrRequestTxn(), *pReqOp->mutable_request_txn());
		}
	}

	void EtcdClient::EtcdClientImpl::CopyResponseOpVector(const ::google::protobuf::RepeatedPtrField< ::etcdserverpb::ResponseOp >& Input, std::vector<ResponseOp>& Output)
	{
		for (int i = 0; i < Input.size(); i++)
		{
			const auto& resEle = Input[i];

			ResponseOp resOp;

			if (resEle.has_response_range()&& nullptr != resOp.GetsPtrResponseRang())
			{
				CopyRangResponse(resEle.response_range(), *resOp.GetsPtrResponseRang());
			}

			if (resEle.has_response_put() && nullptr != resOp.GetsPtrResponsePut())
			{
				CopyPutResponse(resEle.response_put(), *resOp.GetsPtrResponsePut());
			}

			if (resEle.has_response_delete_range() && nullptr != resOp.GetsPtrResponseDeleteRange())
			{
				CopyDeleteRangeResponse(resEle.response_delete_range(), *resOp.GetsPtrResponseDeleteRange());
			}

			if (resEle.has_response_txn() && nullptr != resOp.GetsPtrResponseTxn())
			{
				CopyTxnResponse(resEle.response_txn(),*resOp.GetsPtrResponseTxn());
			}
		}
	}

	void EtcdClient::EtcdClientImpl::CopyTxnResponse(const etcdserverpb::TxnResponse& resgrpc, TxnResponse& response)
	{
		if (resgrpc.has_header())
			CopyHeader(resgrpc.header(), response.header);

		response.bSucesseded = resgrpc.succeeded();

		CopyResponseOpVector(resgrpc.responses(),response.vResponses);
	}

	int EtcdClient::EtcdClientImpl::Compact(const CompactionRequest& request, CompactionResponse& response)
	{
		if (nullptr == m_uPtrKVImpl)
			return E_G_VALUE::KV_NO_INIT;

		::etcdserverpb::CompactionRequest  reqgrpc;
		::etcdserverpb::CompactionResponse resgrpc;

		if (!request.nRevision.Empty())
			reqgrpc.set_revision(request.nRevision.GetInt());

		if (!request.bPhysical.Empty())
			reqgrpc.set_physical(request.bPhysical.GetBool());

		int nRet = m_uPtrKVImpl->Compact(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::Watch(WatchCallBackBase* pClassCallBack /*= nullptr*/)
	{
		if (nullptr == m_sPtrWathImpl)
			return E_G_VALUE::WATCH_NO_INIT;

		int nRet = m_sPtrWathImpl->Watch();
		if (E_G_VALUE::SUCCESS != nRet)
			return nRet;

		m_sPtrWathImpl->SetpWatchCallBackBase(pClassCallBack);

		return E_G_VALUE::SUCCESS;
	}

	void EtcdClient::EtcdClientImpl::CopyEvent(const ::mvccpb::Event& input, Event& output)
	{
		output.type = (Event::EventType)input.type();

		if (input.has_kv())
		{
			EtcdClient::EtcdClientImpl::CopyKeyValue(input.kv(), output.kv);
		}

		if (input.has_prev_kv())
		{
			EtcdClientImpl::CopyKeyValue(input.prev_kv(), output.preKv);
		}
		
	}

	void EtcdClient::EtcdClientImpl::CopyEventVector(const ::google::protobuf::RepeatedPtrField< ::mvccpb::Event >& input, vector<Event >& output)
	{
		for (int i = 0; i < input.size(); i++)
		{
			Event evt;
			EtcdClient::EtcdClientImpl::CopyEvent(input[i], evt);
			output.push_back(std::move(evt));
		}
	}

	int EtcdClient::EtcdClientImpl::Write(const WatchRequest& request)
	{
		if (nullptr == m_sPtrWathImpl)
			return E_G_VALUE::WATCH_NO_INIT;

		if (request.Empty())
			return E_G_VALUE::INPUT_ERR;

		::etcdserverpb::WatchRequest reqgrpc;

		auto sPtrWatchCreate = request.GetcsPtrWatchCreateRequest();
		if (nullptr != sPtrWatchCreate)
		{
			auto *pCreateReq = reqgrpc.mutable_create_request();
			if (sPtrWatchCreate->bAllKey)
			{
				pCreateReq->set_key("", 1);
				pCreateReq->set_range_end("", 1);
			}
			else
			{
				if(sPtrWatchCreate->sKey.Empty())
					return E_G_VALUE::INPUT_KEY_ERR;

				pCreateReq->set_key(sPtrWatchCreate->sKey.GetString());

				if (sPtrWatchCreate->bEqOrGre)
				{
					pCreateReq->set_range_end("", 1);
				}
				else if (sPtrWatchCreate->bPrefixed)
				{
					pCreateReq->set_range_end(EtcdClientImpl::RetAddOneKey(sPtrWatchCreate->sKey.GetString()));
				}
				else
				{
					if (!sPtrWatchCreate->sRangeEnd.Empty())
						pCreateReq->set_range_end(sPtrWatchCreate->sRangeEnd.GetString());
				}
			}

			if (!sPtrWatchCreate->bProgressNotify.Empty())
				pCreateReq->set_progress_notify(sPtrWatchCreate->bProgressNotify.GetBool());

			for (std::vector<WatchCreateRequest::FilterType>::const_iterator iter = sPtrWatchCreate->vFilters.cbegin();
				iter != sPtrWatchCreate->vFilters.cend();++iter)
			{
			   pCreateReq->add_filters((::etcdserverpb::WatchCreateRequest_FilterType)*iter);
			}

			if (!sPtrWatchCreate->bPreKv.Empty())
				pCreateReq->set_prev_kv(sPtrWatchCreate->bPreKv.GetBool());
		}


		auto sPtrCancelReq = request.GetcsPtrWatchCancelRequest();
		if (nullptr != sPtrCancelReq)
		{
			auto pCancelReq = reqgrpc.mutable_cancel_request();

			if(!sPtrCancelReq->nWatchId.Empty())
				pCancelReq->set_watch_id(sPtrCancelReq->nWatchId.GetInt());
		}
		
		return m_sPtrWathImpl->Write(reqgrpc);
	}

	int EtcdClient::EtcdClientImpl::LeaseGrant(const LeaseGrantRequest& request, LeaseGrantResponse& response)
	{
		if (nullptr == m_uPtrLeaseImpl)
			return E_G_VALUE::LEASE_NO_INIT;

		etcdserverpb::LeaseGrantRequest  reqgrpc;
		etcdserverpb::LeaseGrantResponse resgrpc;

		if (!request.nID.Empty())
			reqgrpc.set_id(request.nID.GetInt());

		if (!request.nTTL.Empty())
			reqgrpc.set_ttl(request.nTTL.GetInt());

		int nRet = m_uPtrLeaseImpl->LeaseGrant(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		response.nID    = resgrpc.id();
		response.nTTL   = resgrpc.ttl();
		response.sError = resgrpc.error();

		return E_G_VALUE::SUCCESS;
	}

	void EtcdClient::EtcdClientImpl::InitLease(const string& sIpAndPor)
	{
		//m_uPtrLeaseImpl = std::make_unique<LeaseImpl>(sIpAndPor);
		std::unique_ptr<LeaseImpl> uPtrLease = std::unique_ptr<LeaseImpl>(new LeaseImpl(sIpAndPor));
		m_uPtrLeaseImpl.swap(uPtrLease);
	}

	void EtcdClient::EtcdClientImpl::InitAuth(const string & sIpAndPor)
	{
		std::unique_ptr<AuthImpl> uPtrAuth = std::unique_ptr<AuthImpl>(new AuthImpl(sIpAndPor));
		m_uPtrAuthImpl.swap(uPtrAuth);
	}

	int EtcdClient::EtcdClientImpl::LeaseRevoke(const LeaseRevokeRequest& request, LeaseRevokeResponse& response)
	{
		if (nullptr == m_uPtrLeaseImpl)
			return E_G_VALUE::LEASE_NO_INIT;

		etcdserverpb::LeaseRevokeRequest reqgrpc;
		etcdserverpb::LeaseRevokeResponse resgrpc;

		if (!request.nID.Empty())
			reqgrpc.set_id(request.nID.GetInt());

		int nRet = m_uPtrLeaseImpl->LeaseRevoke(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::LeaseKeepAlive(LeaseCallBackBase* pInput /*= nullptr*/)
	{
		if (nullptr == m_uPtrLeaseImpl)
			return E_G_VALUE::LEASE_NO_INIT;

		int nRet = m_uPtrLeaseImpl->LeaseKeepAlive();
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		m_uPtrLeaseImpl->SetpLeaseCallBackBase(pInput);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::LeaseKeepAliveWrite(const LeaseKeepAliveRequest& request)
	{
		if (nullptr == m_uPtrLeaseImpl)
			return E_G_VALUE::LEASE_NO_INIT;

		etcdserverpb::LeaseKeepAliveRequest reqgrpc;

		if (!request.nID.Empty())
			reqgrpc.set_id(request.nID.GetInt());

		int nRet = m_uPtrLeaseImpl->LeaseKeepAliveWrite(reqgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::LeaseTimeToLive(const LeaseTimeToLiveRequest& request, LeaseTimeToLiveResponse& response)
	{
		if (nullptr == m_uPtrLeaseImpl)
			return E_G_VALUE::LEASE_NO_INIT;

		etcdserverpb::LeaseTimeToLiveRequest  reqgrpc;
		etcdserverpb::LeaseTimeToLiveResponse resgrpc;

		if (!request.nID.Empty())
			reqgrpc.set_id(request.nID.GetInt());

		if (!request.bKeys.Empty())
			reqgrpc.set_keys(request.bKeys.GetBool());

		int nRet = m_uPtrLeaseImpl->LeaseTimeToLive(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		response.nID = resgrpc.id();
		response.nTTL = resgrpc.ttl();
		response.nGrantedTTL = resgrpc.grantedttl();
		
		for (int i = 0; i < resgrpc.keys_size(); i++)
		{
			response.vKeys.push_back(resgrpc.keys(i));
		}

		return E_G_VALUE::SUCCESS;
	}

	const bool EtcdClient::EtcdClientImpl::GetbWatchNormal() const
	{
		if(nullptr != m_sPtrWathImpl)
		    return m_sPtrWathImpl->GetbWatchNormal();

		return false;
	}

	void EtcdClient::EtcdClientImpl::SetnIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen)
	{
		if (nullptr != m_sPtrWathImpl)
			m_sPtrWathImpl->SetnIntervalAndnMaxDataLen(nInterval, nMaxDataLen);
	}

	const bool EtcdClient::EtcdClientImpl::GetbLeaseNormal() const
	{
		if (nullptr != m_uPtrLeaseImpl)
			return m_uPtrLeaseImpl->GetbLeaseNormal();

		return false;
	}

	void EtcdClient::EtcdClientImpl::TakeResponse(std::shared_ptr<LeaseKeepAliveResponse>& sPtrResponse)
	{
		if (nullptr != m_uPtrLeaseImpl)
			m_uPtrLeaseImpl->TakeResponse(sPtrResponse);
	}

	void EtcdClient::EtcdClientImpl::TakResponses(std::list<std::shared_ptr<LeaseKeepAliveResponse>>& outRespons)
	{
		if (nullptr != m_uPtrLeaseImpl)
			m_uPtrLeaseImpl->TakResponses(outRespons);
	}

	void EtcdClient::EtcdClientImpl::SetLeasenIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen)
	{
		if (nullptr != m_uPtrLeaseImpl)
			m_uPtrLeaseImpl->SetnIntervalAndnMaxDataLen(nInterval,  nMaxDataLen);
	}

	void EtcdClient::EtcdClientImpl::SetLeasebStoreRes(bool bStore)
	{
		if (nullptr != m_uPtrLeaseImpl)
			m_uPtrLeaseImpl->SetbStoreRes(bStore);
	}

	bool EtcdClient::EtcdClientImpl::LeaseResponsesEmpty()
	{
		if (nullptr != m_uPtrLeaseImpl)
			return m_uPtrLeaseImpl->ResponsesEmpty();

		return true;
	}

	int EtcdClient::EtcdClientImpl::AuthEnable(const AuthEnableRequest & request, AuthEnableResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthEnableRequest reqgrpc;
		::etcdserverpb::AuthEnableResponse resgrpc;

		int nRet = m_uPtrAuthImpl->AuthEnable(reqgrpc,resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);


		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::AuthDisable(const AuthDisableRequest & request, AuthDisableResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthDisableRequest reqgrpc;
		::etcdserverpb::AuthDisableResponse resgrpc;

		int nRet = m_uPtrAuthImpl->AuthDisable(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::Authenticate(const AuthenticateRequest & request, AuthenticateResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthenticateRequest  reqgrpc;
		::etcdserverpb::AuthenticateResponse resgrpc;

		if (!request.sName.Empty())
			reqgrpc.set_name(request.sName.GetString());

		if (!request.sPassWord.Empty())
			reqgrpc.set_password(request.sPassWord.GetString());

		int nRet = m_uPtrAuthImpl->Authenticate(reqgrpc, resgrpc);

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		response.sToken = resgrpc.token();


		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::UserAdd(const AuthUserAddRequest & request, AuthUserAddResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthUserAddRequest  reqgrpc;
		::etcdserverpb::AuthUserAddResponse resgrpc;

		if (!request.sName.Empty())
			reqgrpc.set_name(request.sName.GetString());

		if (!request.sPassWord.Empty())
			reqgrpc.set_password(request.sPassWord.GetString());

		int nRet = m_uPtrAuthImpl->UserAdd(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::UserGet(const AuthUserGetRequest & request, AuthUserGetResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthUserGetRequest  reqgrpc;
		::etcdserverpb::AuthUserGetResponse resgrpc;

		if (!request.sName.Empty())
			reqgrpc.set_name(request.sName.GetString());

		int nRet = m_uPtrAuthImpl->UserGet(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		for (int i = 0; i < resgrpc.roles_size(); i++)
		{
			response.vRoles.push_back(resgrpc.roles(i));
		}

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::UserList(const AuthUserListRequest & request, AuthUserListResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthUserListRequest  reqgrpc;
		::etcdserverpb::AuthUserListResponse resgrpc;

		int nRet = m_uPtrAuthImpl->UserList(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);


		for (int i = 0; i < resgrpc.users_size(); i++)
		{
			response.vUsers.push_back(resgrpc.users(i));
		}

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::UserDelete(const AuthUserDeleteRequest & request, AuthUserDeleteResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthUserDeleteRequest  reqgrpc;
		::etcdserverpb::AuthUserDeleteResponse resgrpc;

		if (!request.sName.Empty())
			reqgrpc.set_name(request.sName.GetString());

		int nRet = m_uPtrAuthImpl->UserDelete(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::UserChangePassword(const AuthUserChangePasswordRequest & request, AuthUserChangePasswordResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthUserChangePasswordRequest  reqgrpc;
		::etcdserverpb::AuthUserChangePasswordResponse resgrpc;

		if (!request.sName.Empty())
			reqgrpc.set_name(request.sName.GetString());

		if (!request.sPassWord.Empty())
			reqgrpc.set_password(request.sPassWord.GetString());

		int nRet = m_uPtrAuthImpl->UserChangePassword(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::UserGrantRole(const AuthUserGrantRoleRequest & request, AuthUserGrantRoleResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthUserGrantRoleRequest  reqgrpc;
		::etcdserverpb::AuthUserGrantRoleResponse resgrpc;

		if (!request.sUser.Empty())
			reqgrpc.set_user(request.sUser.GetString());

		if (!request.sRole.Empty())
			reqgrpc.set_role(request.sRole.GetString());

		int nRet = m_uPtrAuthImpl->UserGrantRole(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::UserRevokeRole(const AuthUserRevokeRoleRequest & request, AuthUserRevokeRoleResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthUserRevokeRoleRequest  reqgrpc;
		::etcdserverpb::AuthUserRevokeRoleResponse resgrpc;

		if (!request.sName.Empty())
			reqgrpc.set_name(request.sName.GetString());

		if (!request.sRole.Empty())
			reqgrpc.set_role(request.sRole.GetString());

		int nRet = m_uPtrAuthImpl->UserRevokeRole(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::RoleAdd(const AuthRoleAddRequest & request, AuthRoleAddResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthRoleAddRequest  reqgrpc;
		::etcdserverpb::AuthRoleAddResponse resgrpc;

		if (!request.sName.Empty())
			reqgrpc.set_name(request.sName.GetString());

		int nRet = m_uPtrAuthImpl->RoleAdd(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::RoleGet(const AuthRoleGetRequest & request, AuthRoleGetResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthRoleGetRequest  reqgrpc;
		::etcdserverpb::AuthRoleGetResponse resgrpc;

		if (!request.sRole.Empty())
			reqgrpc.set_role(request.sRole.GetString());

		int nRet = m_uPtrAuthImpl->RoleGet(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		for (int i = 0; i < resgrpc.perm_size();i++)
		{
			Permission perm;
			perm.ePermType = (Permission::Type)resgrpc.perm(i).permtype();
			perm.sKey = resgrpc.perm(i).key();
			perm.sRangeEnd = resgrpc.perm(i).range_end();

			response.vPerm.push_back(::move(perm));
		}

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::RoleList(const AuthRoleListRequest & request, AuthRoleListResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthRoleListRequest  reqgrpc;
		::etcdserverpb::AuthRoleListResponse resgrpc;


		int nRet = m_uPtrAuthImpl->RoleList(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		for (int i = 0; i < resgrpc.roles_size(); i++)
		{
			response.vRoles.push_back(resgrpc.roles(i));
		}

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::RoleDelete(const AuthRoleDeleteRequest & request, AuthRoleDeleteResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthRoleDeleteRequest  reqgrpc;
		::etcdserverpb::AuthRoleDeleteResponse resgrpc;

		if (!request.sRole.Empty())
			reqgrpc.set_role(request.sRole.GetString());

		int nRet = m_uPtrAuthImpl->RoleDelete(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::RoleGrantPermission(const AuthRoleGrantPermissionRequest & request, AuthRoleGrantPermissionResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthRoleGrantPermissionRequest  reqgrpc;
		::etcdserverpb::AuthRoleGrantPermissionResponse resgrpc;

		if (!request.sName.Empty())
			reqgrpc.set_name(request.sName.GetString());

		if (!request.permValue.ePermType.Empty())
			reqgrpc.mutable_perm()->set_permtype((::authpb::Permission_Type)request.permValue.ePermType.GetInt());

		if (!request.permValue.sKey.Empty())
			reqgrpc.mutable_perm()->set_key(request.permValue.sKey.GetString());

		if (!request.permValue.sRangeEnd.Empty())
			reqgrpc.mutable_perm()->set_range_end(request.permValue.sRangeEnd.GetString());


		int nRet = m_uPtrAuthImpl->RoleGrantPermission(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	int EtcdClient::EtcdClientImpl::RoleRevokePermission(const AuthRoleRevokePermissionRequest & request, AuthRoleRevokePermissionResponse & response)
	{
		if (nullptr == m_uPtrAuthImpl)
			return E_G_VALUE::AUTH_NO_INIT;

		::etcdserverpb::AuthRoleRevokePermissionRequest  reqgrpc;
		::etcdserverpb::AuthRoleRevokePermissionResponse resgrpc;

		if (!request.sRole.Empty())
			reqgrpc.set_role(request.sRole.GetString());

		if (!request.sKey.Empty())
			reqgrpc.set_key(request.sKey.GetString());

		if (!request.sRangeEnd.Empty())
			reqgrpc.set_range_end(request.sRangeEnd.GetString());

		int nRet = m_uPtrAuthImpl->RoleRevokePermission(reqgrpc, resgrpc);
		if (E_G_VALUE::SUCCESS != nRet)
		{
			return nRet;
		}

		if (resgrpc.has_header())
			EtcdClientImpl::CopyHeader(resgrpc.header(), response.header);

		return E_G_VALUE::SUCCESS;
	}

	EtcdClient::EtcdClientImpl::WatchImpl::WatchImpl(const string& sIpAndPort)
	{
		m_sIpAndPort = sIpAndPort;
		m_stubWatch = ::etcdserverpb::Watch::NewStub(grpc::CreateChannel(sIpAndPort, grpc::InsecureChannelCredentials()));
		m_bWatchNormal.exchange(true);
		m_nInterval.exchange(3);
		m_nMaxDataLen.exchange(2000);
	}

	EtcdClient::EtcdClientImpl::WatchImpl::~WatchImpl()
	{
		Stop();
	}

	int EtcdClient::EtcdClientImpl::WatchImpl::Watch()
	{
		if (m_bInitWatch)
			return E_G_VALUE::WATCH_HAVED;
	
		m_sPtrClientContext = std::make_shared<ClientContext>();

		m_uPtrClientRW = m_stubWatch->Watch(m_sPtrClientContext.get());
		if (nullptr == m_uPtrClientRW)
		{
			return E_G_VALUE::WATCH_FAIL;
		}
		m_bInitWatch=true;

		m_bRuning.exchange(true);
		m_thread = std::make_shared<thread>(&EtcdClientImpl::WatchImpl::RunInThread, this);

		return E_G_VALUE::SUCCESS;
	}


	void EtcdClient::EtcdClientImpl::WatchImpl::TakeResponse(std::shared_ptr<WatchResponse>& sPtrResponse)
	{
		if (ResponsesEmpty())
			return;

		sPtrResponse.reset();

		std::unique_lock<mutex> locker(m_mutexData);
		sPtrResponse.swap(m_multiMapResPonse.begin()->second);
		m_multiMapResPonse.erase(m_multiMapResPonse.begin());
	}

	void EtcdClient::EtcdClientImpl::WatchImpl::TakResponses(std::map<int64,vector<std::shared_ptr<WatchResponse> > >& outRespons)
	{
		outRespons.clear();

		std::multimap<chrono::time_point<chrono::high_resolution_clock>, std::shared_ptr<WatchResponse> > multiMapRes;

		{
			std::unique_lock<mutex> locker(m_mutexData);
			multiMapRes.swap(m_multiMapResPonse);
		}

		for (const auto& ele:multiMapRes)
		{
			int64 nWatchId = ele.second->nWatchId;

			auto iter = outRespons.find(nWatchId);
			if (outRespons.end() == iter)
				iter = outRespons.insert(make_pair(nWatchId, vector<std::shared_ptr<WatchResponse> >())).first;

			iter->second.push_back(std::move( ele.second));
		}
	}

	void EtcdClient::EtcdClientImpl::WatchImpl::TakResponses(std::list<std::shared_ptr<WatchResponse> >& outRespons)
	{
		outRespons.clear();

		std::multimap<chrono::time_point<chrono::high_resolution_clock>, std::shared_ptr<WatchResponse> > multiMapRes;

		{
			std::unique_lock<mutex> locker(m_mutexData);
			multiMapRes.swap(m_multiMapResPonse);
		}

		for (const auto& ele : multiMapRes)
		{
			outRespons.push_back(std::move( ele.second));
		}
	}

	void EtcdClient::EtcdClientImpl::WatchImpl::SetpWatchCallBackBase(WatchCallBackBase* pInput)
	{
		std::unique_lock<mutex> locker(m_mutexData);
		m_pWatchCallBackBase = pInput;
	}

	void EtcdClient::EtcdClientImpl::WatchImpl::RunInThread()
	{

		::etcdserverpb::WatchResponse  resgrpc;

		while (m_bRuning && nullptr!=m_uPtrClientRW)
		{
			resgrpc.Clear();

			if (m_uPtrClientRW->Read(&resgrpc))
			{
	
#if _PRINT_RES_STRING_
				std::string sRe = resgrpc.DebugString();
				resgrpc.PrintDebugString();
#endif

				shared_ptr<WatchResponse> sPtrRes = make_shared<WatchResponse>();
				CopyWatchResponse(resgrpc, *sPtrRes);

				if(m_bStoreRes)
				{
					unique_lock<mutex> locker(m_mutexData);
					m_multiMapResPonse.insert(make_pair(chrono::high_resolution_clock::now(),sPtrRes));
				}

				WatchCallBackBase* pCallBack = nullptr;
				{
					unique_lock<mutex> locker(m_mutexData);
					pCallBack = m_pWatchCallBackBase;
				}

				if (nullptr != pCallBack)
					pCallBack->RunAfterReadInWatch(*sPtrRes);
			}
			else
			{
				m_bWatchNormal.exchange(false);
			}
		
			while(m_bStoreRes)
			{
				shared_ptr<WatchResponse> sPtrResErase;

				unique_lock<mutex> locker(m_mutexData);
				if(!m_multiMapResPonse.empty())
				{
					std::multimap<chrono::time_point<chrono::high_resolution_clock>, shared_ptr<WatchResponse>  >::iterator  iter = m_multiMapResPonse.begin();
					
					int64 nDiff = chrono::duration_cast<chrono::minutes>(chrono::high_resolution_clock::now() - iter->first).count();
					if ((int)nDiff >= m_nInterval.load() || (int)m_multiMapResPonse.size() >= m_nMaxDataLen.load())
					{

						sPtrResErase.swap(iter->second);
						m_multiMapResPonse.erase(iter);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
				
			}

			this_thread::sleep_for(chrono::milliseconds(100));
		}

		cout << "tys test fffff " << __FUNCTION__ << endl;
	}

	int EtcdClient::EtcdClientImpl::WatchImpl::Write(const etcdserverpb::WatchRequest& request)
	{
		if (nullptr == m_uPtrClientRW)
			return E_G_VALUE::WATCH_NO_INIT;

		if (m_uPtrClientRW->Write(request))
			return E_G_VALUE::SUCCESS;

		m_bWatchNormal.exchange(false);
		return E_G_VALUE::WATCH_WRITE_ERR;
	}

	void EtcdClient::EtcdClientImpl::WatchImpl::SetnIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen) 
	{
		m_nInterval.store(nInterval);
		m_nMaxDataLen.store(nMaxDataLen);
	}

	void EtcdClient::EtcdClientImpl::WatchImpl::CopyWatchResponse(const etcdserverpb::WatchResponse& Input, WatchResponse& Output)
	{
		if (Input.has_header())
			EtcdClient::EtcdClientImpl::CopyHeader(Input.header(), Output.header);

		Output.nWatchId         = Input.watch_id()         ;
		Output.bCreated         = Input.created()          ;
		Output.bCanceled        = Input.canceled()         ;
		Output.nCompactRevision = Input.compact_revision() ;
		Output.sCancelReson     = Input.cancel_reason()    ;

		EtcdClient::EtcdClientImpl::CopyEventVector(Input.events(),Output.vEvents);
	}

	void EtcdClient::EtcdClientImpl::WatchImpl::Stop()
	{
		m_bRuning.store(false);

		if (nullptr != m_uPtrClientRW)
			m_uPtrClientRW->WritesDone();

		if (nullptr != m_thread && m_thread->joinable())
			m_thread->join();
	}

	bool EtcdClient::EtcdClientImpl::WatchImpl::ResponsesEmpty()
	{
		unique_lock<mutex> locker(m_mutexData);
		return m_multiMapResPonse.empty();
	}

	EtcdClient::EtcdClientImpl::KVImpl::KVImpl(const string& sIpAndPort)
	{
		m_sIpAndPort = sIpAndPort;
		m_stubKV = ::etcdserverpb::KV::NewStub(grpc::CreateChannel(sIpAndPort,grpc::InsecureChannelCredentials()));
	}

	EtcdClient::EtcdClientImpl::KVImpl::~KVImpl()
	{

	}

	const string& EtcdClient::EtcdClientImpl::KVImpl::GetsIpAndPort() const
	{
		return m_sIpAndPort;
	}

	int EtcdClient::EtcdClientImpl::KVImpl::Range(const etcdserverpb::RangeRequest& request, etcdserverpb::RangeResponse& response)
	{
		ClientContext context;
		Status status = m_stubKV->Range(&context, request, &response);
		if (status.ok())
		{
#if _PRINT_RES_STRING_
			std::string sRe = response.DebugString();
			response.PrintDebugString();
			auto sKvres = response.kvs_size();
#endif
			return E_G_VALUE::SUCCESS;
		}
		else
		{
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;

			return E_G_VALUE::RPC_RET_ERR;
		}
	}

	int EtcdClient::EtcdClientImpl::KVImpl::Put(const etcdserverpb::PutRequest& request, etcdserverpb::PutResponse& response)
	{
		ClientContext context;
		Status status = m_stubKV->Put(&context, request, &response);
		if (status.ok())
		{
#if _PRINT_RES_STRING_
			std::string sRe = response.DebugString();
			response.PrintDebugString();
#endif
			return E_G_VALUE::SUCCESS;
		}
		else
		{
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;

			return E_G_VALUE::RPC_RET_ERR;
		}
	}

	int EtcdClient::EtcdClientImpl::KVImpl::DeleteRange(const etcdserverpb::DeleteRangeRequest& request, etcdserverpb::DeleteRangeResponse& response)
	{
		ClientContext context;
		Status status = m_stubKV->DeleteRange(&context, request, &response);
		if (status.ok())
		{
#if _PRINT_RES_STRING_
			std::string sRe = response.DebugString();
			response.PrintDebugString();
#endif
			return E_G_VALUE::SUCCESS;
		}
		else
		{
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;

			return E_G_VALUE::RPC_RET_ERR;
		}
	}

	int EtcdClient::EtcdClientImpl::KVImpl::Txn(const etcdserverpb::TxnRequest& request, etcdserverpb::TxnResponse& response)
	{
		ClientContext context;
		Status status = m_stubKV->Txn(&context, request, &response);
		if (status.ok())
		{
#if _PRINT_RES_STRING_
			std::string sRe = response.DebugString();
			response.PrintDebugString();
#endif
			return E_G_VALUE::SUCCESS;
		}
		else
		{
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;

			return E_G_VALUE::RPC_RET_ERR;
		}
	}

	int EtcdClient::EtcdClientImpl::KVImpl::Compact(const etcdserverpb::CompactionRequest& request, etcdserverpb::CompactionResponse& response)
	{
		ClientContext context;
		Status status = m_stubKV->Compact(&context, request, &response);
		if (status.ok())
		{
#if _PRINT_RES_STRING_
			std::string sRe = response.DebugString();
			response.PrintDebugString();
#endif
			return E_G_VALUE::SUCCESS;
		}
		else
		{
			std::cout << status.error_code() << ": " << status.error_message()
				<< std::endl;

			return E_G_VALUE::RPC_RET_ERR;
		}
	}

     EtcdClient::EtcdClient():m_uPtrImpl(new EtcdClientImpl())
	{
	}

	 EtcdClient::~EtcdClient()
	 {

	 }

	 void EtcdClient::InitKV(const string& sIpAndProt)
	 {
		 assert(!sIpAndProt.empty());
		 m_uPtrImpl->InitKVImpl(sIpAndProt);
	 }

	 void EtcdClient::InitWatch(const string& sIpAndProt)
	 {
		 assert(!sIpAndProt.empty());
		 m_uPtrImpl->InitWatch(sIpAndProt);
	 }

	 void EtcdClient::InitLease(const string & sIpAndPort)
	 {
		 assert(!sIpAndPort.empty());
		 return m_uPtrImpl->InitLease(sIpAndPort);
	 }

	 string EtcdClient::GetKVIpAndPort() const
{
		 return m_uPtrImpl->GetKVIpAndPort();
	 }

	 string EtcdClient::GetWatchIpandPort() const
{
		 return m_uPtrImpl->GetWatchIpAndProt();
	 }

	 int EtcdClient::Range(const RangeRequest& request, RangeResponse& response)
	 {
		 return m_uPtrImpl->Range(request,response);
	 }

	 int EtcdClient::Put(const PutRequest& request, PutResponse& response)
	 {
		 return m_uPtrImpl->Put(request, response);
	 }

	 int EtcdClient::DeleteRange(const DeleteRangeRequest& request, DeleteRangeResponse& response)
	 {
		 return m_uPtrImpl->DeleteRange(request, response);
	 }

	 int EtcdClient::Txn(const TxnRequest& request, TxnResponse& response)
	 {
		 return m_uPtrImpl->Txn(request, response);
	 }

	 int EtcdClient::Compact(const CompactionRequest& request, CompactionResponse& response)
	 {
		 return m_uPtrImpl->Compact(request, response);
	 }

	 int EtcdClient::Watch(WatchCallBackBase* pClassCallBack /*=nullptr*/)
	 {
		 return m_uPtrImpl->Watch(pClassCallBack);
	 }

	 void   EtcdClient::TakeResponse(std::shared_ptr<WatchResponse>& sPtrResponse)
	 {
		 if (nullptr != m_uPtrImpl->GetsPtrWathImpl())
			 m_uPtrImpl->GetsPtrWathImpl()->TakeResponse(sPtrResponse);
	 }

	 void EtcdClient::TakResponses(std::map<int64, vector<std::shared_ptr<WatchResponse> > >& outRespons)
	 {
		 if (nullptr != m_uPtrImpl->GetsPtrWathImpl())
			 m_uPtrImpl->GetsPtrWathImpl()->TakResponses(outRespons);
	 }

	 void EtcdClient::TakResponses(std::list<std::shared_ptr<WatchResponse> >& outRespons)
	 {
		 if (nullptr != m_uPtrImpl->GetsPtrWathImpl())
			 m_uPtrImpl->GetsPtrWathImpl()->TakResponses(outRespons);
	 }

	 bool EtcdClient::ResponsesEmpty()
	 {
		 if (nullptr != m_uPtrImpl->GetsPtrWathImpl())
			 return m_uPtrImpl->GetsPtrWathImpl()->ResponsesEmpty();

		 return true;
	 }

	 bool EtcdClient::GetbWatchNormal()
	 {
		 if (nullptr != m_uPtrImpl->GetsPtrWathImpl())
			 return m_uPtrImpl->GetsPtrWathImpl()->GetbWatchNormal();

		 return false;
	 }

	 int EtcdClient::Write(const WatchRequest& request)
	 {
		 return m_uPtrImpl->Write(request);
	 }

	 void EtcdClient::SetnIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen)
	 {
		 if (nullptr != m_uPtrImpl->GetsPtrWathImpl())
			 m_uPtrImpl->GetsPtrWathImpl()->SetnIntervalAndnMaxDataLen(nInterval, nMaxDataLen);
	 }

	 void EtcdClient::SetbStore(bool bInput /*= false*/)
	 {
		 if (nullptr != m_uPtrImpl->GetsPtrWathImpl())
			 m_uPtrImpl->GetsPtrWathImpl()->SetbStore(bInput);
	 }

	 const bool EtcdClient::GetbStore() const
	 {
		 if (nullptr != m_uPtrImpl->GetsPtrWathImpl())
			 return m_uPtrImpl->GetsPtrWathImpl()->GetbStore();

		 return false;
	 }

	 int EtcdClient::LeaseGrant(const LeaseGrantRequest& request, LeaseGrantResponse& response)
	 {
		 return m_uPtrImpl->LeaseGrant(request, response);
	 }

	 int EtcdClient::LeaseRevoke(const LeaseRevokeRequest& request, LeaseRevokeResponse& response)
	 {
		 return m_uPtrImpl->LeaseRevoke(request, response);
	 }

	 int EtcdClient::LeaseKeepAlive(LeaseCallBackBase* pCallBackClass)
	 {
		 return m_uPtrImpl->LeaseKeepAlive(pCallBackClass);
	 }

	 int EtcdClient::LeaseKeepAliveWrite(const LeaseKeepAliveRequest& request)
	 {
		 return m_uPtrImpl->LeaseKeepAliveWrite(request);
	 }

	 bool EtcdClient::LeaseResponsesEmpty()
	 {
		 return m_uPtrImpl->LeaseResponsesEmpty();
	 }

	 bool EtcdClient::GetbLeaseNormal()
	 {
		 return m_uPtrImpl->GetbLeaseNormal();
	 }

	 void EtcdClient::TakeResponse(std::shared_ptr<LeaseKeepAliveResponse>& sPtrResponse)
	 {
		 return m_uPtrImpl->TakeResponse(sPtrResponse);
	 }

	 void EtcdClient::TakResponses(std::list<std::shared_ptr<LeaseKeepAliveResponse> >& outRespons)
	 {
		 return m_uPtrImpl->TakResponses(outRespons);
	 }

	 void EtcdClient::SetLeasenIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen)
	 {
		 return m_uPtrImpl->SetLeasenIntervalAndnMaxDataLen(nInterval, nMaxDataLen);
	 }

	 void EtcdClient::SetLeasebStoreRes(bool bStore)
	 {
		 return m_uPtrImpl->SetLeasebStoreRes(bStore);
	 }

	 int EtcdClient::AuthEnable(const AuthEnableRequest & request, AuthEnableResponse & response)
	 {
		 return m_uPtrImpl->AuthEnable(request, response);
	 }

	 int EtcdClient::AuthDisable(const AuthDisableRequest & request, AuthDisableResponse & response)
	 {
		 return m_uPtrImpl->AuthDisable(request, response);
	 }

	 int EtcdClient::Authenticate(const AuthenticateRequest & request, AuthenticateResponse & response)
	 {
		 return m_uPtrImpl->Authenticate(request, response);
	 }

	 int EtcdClient::UserAdd(const AuthUserAddRequest & request, AuthUserAddResponse & response)
	 {
		 return m_uPtrImpl->UserAdd(request, response);
	 }

	 int EtcdClient::UserGet(const AuthUserGetRequest & request, AuthUserGetResponse & response)
	 {
		 return m_uPtrImpl->UserGet(request, response);
	 }

	 int EtcdClient::UserList(const AuthUserListRequest & request, AuthUserListResponse & response)
	 {
		 return m_uPtrImpl->UserList(request, response);
	 }

	 int EtcdClient::UserDelete(const AuthUserDeleteRequest & request, AuthUserDeleteResponse & response)
	 {
		 return m_uPtrImpl->UserDelete(request, response);
	 }

	 int EtcdClient::UserChangePassword(const AuthUserChangePasswordRequest & request, AuthUserChangePasswordResponse & response)
	 {
		 return m_uPtrImpl->UserChangePassword(request, response);
	 }

	 int EtcdClient::UserGrantRole(const AuthUserGrantRoleRequest & request, AuthUserGrantRoleResponse & response)
	 {
		 return m_uPtrImpl->UserGrantRole(request, response);
	 }

	 int EtcdClient::UserRevokeRole(const AuthUserRevokeRoleRequest & request, AuthUserRevokeRoleResponse & response)
	 {
		 return m_uPtrImpl->UserRevokeRole(request, response);
	 }

	 int EtcdClient::RoleAdd(const AuthRoleAddRequest & request, AuthRoleAddResponse & response)
	 {
		 return m_uPtrImpl->RoleAdd(request, response);
	 }

	 int EtcdClient::RoleGet(const AuthRoleGetRequest & request, AuthRoleGetResponse & response)
	 {
		 return m_uPtrImpl->RoleGet(request, response);
	 }

	 int EtcdClient::RoleList(const AuthRoleListRequest & request, AuthRoleListResponse & response)
	 {
		 return m_uPtrImpl->RoleList(request, response);
	 }

	 int EtcdClient::RoleDelete(const AuthRoleDeleteRequest & request, AuthRoleDeleteResponse & response)
	 {
		 return m_uPtrImpl->RoleDelete(request, response);
	 }

	 int EtcdClient::RoleGrantPermission(const AuthRoleGrantPermissionRequest & request, AuthRoleGrantPermissionResponse & response)
	 {
		 return m_uPtrImpl->RoleGrantPermission(request, response);
	 }

	 int EtcdClient::RoleRevokePermission(const AuthRoleRevokePermissionRequest & request, AuthRoleRevokePermissionResponse & response)
	 {
		 return m_uPtrImpl->RoleRevokePermission(request, response);
	 }

	 int EtcdClient::LeaseTimeToLive(const LeaseTimeToLiveRequest& request, LeaseTimeToLiveResponse& response)
	 {
		 return m_uPtrImpl->LeaseTimeToLive(request,response);
	 }

	 WatchCallBackBase::WatchCallBackBase()
	 {

	 }

	 WatchCallBackBase::~WatchCallBackBase()
	 {

	 }

	 void WatchCallBackBase::RunAfterReadInWatch(const WatchResponse& res)
     {
		 cout << "tys test " << __FUNCTION__ << endl;
	 }

	 EtcdClient::EtcdClientImpl::AuthImpl::AuthImpl(const string & sIpAndPort)
	 {
		 m_sIpAndPort = sIpAndPort;
		 m_stubAuth = ::etcdserverpb::Auth::NewStub(grpc::CreateChannel(sIpAndPort, grpc::InsecureChannelCredentials()));
	 }

	 EtcdClient::EtcdClientImpl::AuthImpl::~AuthImpl()
	 {
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::AuthEnable(const::etcdserverpb::AuthEnableRequest & request, ::etcdserverpb::AuthEnableResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->AuthEnable(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::AuthDisable(const::etcdserverpb::AuthDisableRequest & request, ::etcdserverpb::AuthDisableResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->AuthDisable(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::Authenticate(const::etcdserverpb::AuthenticateRequest & request, ::etcdserverpb::AuthenticateResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->Authenticate(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::UserAdd(const::etcdserverpb::AuthUserAddRequest & request, ::etcdserverpb::AuthUserAddResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->UserAdd(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::UserGet(const::etcdserverpb::AuthUserGetRequest & request, ::etcdserverpb::AuthUserGetResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->UserGet(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::UserList(const::etcdserverpb::AuthUserListRequest & request, ::etcdserverpb::AuthUserListResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->UserList(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::UserDelete(const::etcdserverpb::AuthUserDeleteRequest & request, ::etcdserverpb::AuthUserDeleteResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->UserDelete(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::UserChangePassword(const::etcdserverpb::AuthUserChangePasswordRequest & request, ::etcdserverpb::AuthUserChangePasswordResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->UserChangePassword(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::UserGrantRole(const::etcdserverpb::AuthUserGrantRoleRequest & request, ::etcdserverpb::AuthUserGrantRoleResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->UserGrantRole(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::UserRevokeRole(const::etcdserverpb::AuthUserRevokeRoleRequest & request, ::etcdserverpb::AuthUserRevokeRoleResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->UserRevokeRole(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::RoleAdd(const::etcdserverpb::AuthRoleAddRequest & request, ::etcdserverpb::AuthRoleAddResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->RoleAdd(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::RoleGet(const::etcdserverpb::AuthRoleGetRequest & request, ::etcdserverpb::AuthRoleGetResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->RoleGet(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::RoleList(const::etcdserverpb::AuthRoleListRequest & request, ::etcdserverpb::AuthRoleListResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->RoleList(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::RoleDelete(const::etcdserverpb::AuthRoleDeleteRequest & request, ::etcdserverpb::AuthRoleDeleteResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->RoleDelete(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::RoleGrantPermission(const::etcdserverpb::AuthRoleGrantPermissionRequest & request, ::etcdserverpb::AuthRoleGrantPermissionResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->RoleGrantPermission(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 int EtcdClient::EtcdClientImpl::AuthImpl::RoleRevokePermission(const::etcdserverpb::AuthRoleRevokePermissionRequest & request, ::etcdserverpb::AuthRoleRevokePermissionResponse & response)
	 {
		 ClientContext context;
		 Status status = m_stubAuth->RoleRevokePermission(&context, request, &response);
		 if (status.ok())
		 {
#if _PRINT_RES_STRING_
			 std::string sRe = response.DebugString();
			 response.PrintDebugString();
#endif
			 return E_G_VALUE::SUCCESS;
		 }
		 else
		 {
			 std::cout << status.error_code() << ": " << status.error_message()
				 << std::endl;

			 return E_G_VALUE::RPC_RET_ERR;
		 }
	 }

	 etcdclient::LeaseCallBackBase::LeaseCallBackBase()
	 {

	 }

	 etcdclient::LeaseCallBackBase::~LeaseCallBackBase()
	 {

	 }

	 void etcdclient::LeaseCallBackBase::RunAfterReadInLease(const LeaseKeepAliveResponse& res)
	 {

	 }
}


