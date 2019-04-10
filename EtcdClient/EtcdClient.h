//2018-10-15
//tys create
// etcd client interface class 

#pragma once
#include <memory>
#include "EtcdClientType.h"

namespace etcdclient
{
	class WatchCallBackBase;
	class LeaseCallBackBase;

	class EtcdClient final
	{
	public:

		EtcdClient();
		~EtcdClient();
		//format: "10.0.14.80:2379"
		void InitKV(const string& sIpAndProt);
		void InitWatch(const string& sIpAndProt);
		void InitLease(const string& sIpAndPort);

		string  GetKVIpAndPort() const;
		string  GetWatchIpandPort() const;

		//Range gets the keys in the range from the key-value store.
		int Range(const RangeRequest& request, RangeResponse& response);

		// Put puts the given key into the key-value store.
		// A put request increments the revision of the key-value store
		// and generates one event in the event history.
		int Put(const PutRequest& request,PutResponse& response);

		// DeleteRange deletes the given range from the key-value store.
		// A delete request increments the revision of the key-value store
		// and generates a delete event in the event history for every deleted key.
		int DeleteRange(const DeleteRangeRequest& request, DeleteRangeResponse& response);

		// Txn processes multiple requests in a single transaction.
		// A txn request increments the revision of the key-value store
		// and generates events with the same revision for every completed request.
		// It is not allowed to modify the same key several times within one txn.
		int Txn(const TxnRequest& request, TxnResponse& response);

		//Compact compacts the event history in the etcd key-value store. The key-value
		//store should be periodically compacted or the event history will continue to grow indefinitely
		int Compact(const CompactionRequest& request, CompactionResponse& response);

		//Watch watches for events happening or that that have happened. Both input and output are streams;
		//the input steam is for creating and canceling watchers and the output stream sends events. 
		//One watch PRC can watch on multiply key ranges, streaming events for several watches at once. 
		//The entire event history  can be watched starting form the last compaction revision
		// the input parameter pClassCallBack is the pointer of the child class of WatchCallBackBase,
		// asynchronous call the child class's member function RunAfterReadInWatch after read the reply message 
		int Watch(WatchCallBackBase* pClassCallBack =nullptr);

		// for watch  if SetbStrore is true , can take the response by calling the TakeResponse,TakResponses
		
		void       SetbStore(bool bInput = false);
		const bool GetbStore() const;
		// send the watch request 
		int        Write(const WatchRequest& request);
		// for watch. set the respons's store times and max data length  . 
		//the defult value of nInterval is  3 minutes,the defult value of  nMaxDataLen is 2000.
		void       SetnIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen);

		// if SetbStrore is true , can take the response 
		void       TakeResponse(std::shared_ptr<WatchResponse>& sPtrResponse);
		//map's  key is Watch Id of the created watcher, values are responses .		   
		void       TakResponses(std::map<int64, vector<std::shared_ptr<WatchResponse> > >& outRespons);
		void       TakResponses(std::list<std::shared_ptr<WatchResponse> >& outRespons);
		bool       ResponsesEmpty();
		bool       GetbWatchNormal();

		//LeaseGrant creates a lease which expires if the servers does not receive a keepAlive within
		//g given time to live period. All keys attached to the lease will be expired and deleted if the lease expires.
		// Each expired key generates a delete event in the event history.
		int LeaseGrant(const LeaseGrantRequest& request, LeaseGrantResponse& response);
		//LeaseRevoke revokes a lease. All keys attached to the lease will expired and be deleted.
		int LeaseRevoke(const LeaseRevokeRequest& request, LeaseRevokeResponse& response);
		//LeaseTimeToLive retrieves lease information
		int LeaseTimeToLive(const LeaseTimeToLiveRequest& request, LeaseTimeToLiveResponse& response);
		//LeaseKeepAlive keeps the lease alive by streaming keep alive requests from the client to 
		// the server and streaming keep alive response from the server to the client.
		int LeaseKeepAlive(LeaseCallBackBase* pCallBackClass);
		int LeaseKeepAliveWrite(const LeaseKeepAliveRequest& request);
		bool  LeaseResponsesEmpty();
		bool   GetbLeaseNormal();
		void  TakeResponse(std::shared_ptr<LeaseKeepAliveResponse>& sPtrResponse);
		void  TakResponses(std::list<std::shared_ptr<LeaseKeepAliveResponse> >& outRespons);
		void  SetLeasenIntervalAndnMaxDataLen(int nInterval, int nMaxDataLen);
		void  SetLeasebStoreRes(bool bStore);

		// AuthEnable enables authentication.
		int AuthEnable(const AuthEnableRequest& request, AuthEnableResponse& response);
		// AuthDisable disables authentication.
		int AuthDisable(const AuthDisableRequest& request, AuthDisableResponse& response);
		// Authenticate processes an authenticate request.
		int Authenticate(const AuthenticateRequest& request, AuthenticateResponse& response);
		// UserAdd adds a new user.
		int UserAdd(const AuthUserAddRequest& request, AuthUserAddResponse& response);
		// UserGet gets detailed user information.
		int UserGet(const AuthUserGetRequest& request, AuthUserGetResponse& response);
		// UserList gets a list of all users.
		int UserList(const AuthUserListRequest& request, AuthUserListResponse& response);
		// UserDelete deletes a specified user.
		int UserDelete(const AuthUserDeleteRequest& request, AuthUserDeleteResponse& response);
		// UserChangePassword changes the password of a specified user.
		int UserChangePassword(const AuthUserChangePasswordRequest& request, AuthUserChangePasswordResponse& response);
		// UserGrant grants a role to a specified user.
		int UserGrantRole(const AuthUserGrantRoleRequest& request, AuthUserGrantRoleResponse& response);
		// UserRevokeRole revokes a role of specified user.
		int UserRevokeRole(const AuthUserRevokeRoleRequest& request, AuthUserRevokeRoleResponse& response);
		// RoleAdd adds a new role.
		int RoleAdd(const AuthRoleAddRequest& request, AuthRoleAddResponse& response);
		// RoleGet gets detailed role information.
		int RoleGet(const AuthRoleGetRequest& request, AuthRoleGetResponse& response);
		// RoleList gets lists of all roles.
		int RoleList(const AuthRoleListRequest& request, AuthRoleListResponse& response);
		// RoleDelete deletes a specified role.
		int RoleDelete(const AuthRoleDeleteRequest& request, AuthRoleDeleteResponse& response);
		// RoleGrantPermission grants a permission of a specified key or range to a specified role.
		int RoleGrantPermission(const AuthRoleGrantPermissionRequest& request, AuthRoleGrantPermissionResponse& response);
		// RoleRevokePermission revokes a key or range permission of a specified role.
		int RoleRevokePermission(const AuthRoleRevokePermissionRequest& request, AuthRoleRevokePermissionResponse& response);

	private:

		class EtcdClientImpl;
		unique_ptr<EtcdClientImpl> m_uPtrImpl;
	};

	//WatchResponse is the Watch reply message 
	//realize the virtual member function RunAfterReadInWatch in the child class;
	class WatchCallBackBase
	{
	public:
		WatchCallBackBase();
		virtual ~WatchCallBackBase();
		virtual void RunAfterReadInWatch(const WatchResponse& res);
	};

	class LeaseCallBackBase
	{
	public:
		LeaseCallBackBase();
		virtual ~LeaseCallBackBase();

		virtual void RunAfterReadInLease(const LeaseKeepAliveResponse& res);

	};
}
