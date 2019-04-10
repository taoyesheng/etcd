//2018-10-15
//tys create
// etcd client type

#pragma once
#include <string.h>
#include <memory>
#include <chrono>
#include <cassert>
#include <ctime>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>

#include "xcore_define.h"

using namespace std;

namespace etcdclient
{
#ifndef _PRINT_RES_STRING_
#define _PRINT_RES_STRING_ 1
#endif 

	enum E_G_VALUE
	{
		SUCCESS          = 0  ,
		IP_PORT_ERR      = 1  , //input ip or port err
		RPC_RET_ERR      = 2  ,
		INPUT_ERR        = 3  ,
		INPUT_KEY_ERR    = 4  ,
		WATCH_HAVED      = 5  , //there is already one Watching
		WATCH_FAIL       = 6  ,
		WATCH_WRITE_ERR  = 7  ,
		WATCH_NO_INIT    = 8  ,
		KV_NO_INIT       = 9  ,
		LEASE_HAVED_INIT = 10 ,
		LEASE_NO_LEASING = 11 ,
		LEASE_FAIL       = 12 ,
		LEASE_WRITE_ERR  = 13 ,
		LEASE_NO_INIT    = 14 ,
		AUTH_NO_INIT     = 15 ,
	};

	class Value
	{
	public:

		Value();
		Value(bool b);
		Value(int n);
		Value(unsigned int n);
		Value(long long n);
		Value(float f);
		Value(double d);
		Value(const string& s);
		Value(const char* s);
		Value(const void* ptr, size_t len);
		Value(const Value& hs);
		Value& operator = (const Value& hs);
		Value(Value&& hs);
		Value& operator = (Value&& hs);

		~Value();

		string ToString()const;

		int       GetType() const { return m_type; }
		int       GetLen() const  { return m_len; }
		bool      Empty() const;
		bool      GetBool()const;
		long long GetInt()const;
		double    GetDouble()const;
		string    GetString() const;
		string    GetBinary() const;

	private:

		union Data
		{
			bool      m_bool  ;
			long long m_int   ; // int
			double    m_float ; // float, double
			time_t    m_time  ; // time
			void*     m_ptr   ; // string, binary
		}m_data;

		int    m_type;// 0:null, 1:bool, 2:int64, 3:double, 4:string, 5:binary, 6:time
		size_t m_len;
	};

	template<typename T>
	class DefType
	{
	public:
		bool bHas = false;
		T    value;
	};

	typedef struct _KeyValue
	{
		string sKey                ; // key is the key in bytes. An empty key is not allowed.
		int64  nCreateRevision = 0 ; // create_revision is the revision of last creation on this key.
		int64  nModRevision    = 0 ; // mod_revision is the revision of last modification on this key.
		int64  nVersion        = 0 ; // version is the version of the key. A deletion resets the version to zero and any modification of the key increases its version.
		string sValue              ; // value is the value held by the key, in bytes.
		int64  nLease          = 0 ; // lease is the ID of the lease that attached to key.When the attached lease expires, the key will be deleted.If lease is 0, then no lease is attached to the key.

	}KeyValue;

	typedef struct _Event
	{
		enum EventType {
			E_PUT    = 0,
			E_DELETE = 1,
		};
		//type is the kind of event. of type is a PUT, it indicates
		//new data has been stored to the key. it the type is a DELETE,
		// it indicates the key was deleted.
		EventType type;
		//kv holds the KeyValue for the event.
		//A PUT event contains current kv pair.
		//A PUT event with kv.nVersion=1 indicates the creation of a key.
		//A DELETE/EXPIRE event contains the deleted key with its modification
		//revision set to the revision of deletion.
		DefType<KeyValue> kv;
		//preKv holds the key-value pair before the event happens.
		DefType<KeyValue> preKv;

	}Event;

	typedef struct _Permission
	{
		enum Type {
			READ = 0,
			WRITE = 1,
			READWRITE = 2,
		};
		Type   ePermType ;

		string sKey      ;
		string sRangeEnd ;
	}Permission;

	typedef struct _PermissionValue
	{
		Value ePermType;
		Value sKey;
		Value sRangeEnd;
	}PermissionValue;

	typedef struct  _ResponseHeader
	{
		int64 nClusterId = 0 ; // cluster_id is the ID of the cluster which sent the response.
		int64 nMemberId  = 0 ; // member_id is the ID of the member which sent the response.
		int64 nRevision  = 0 ; // revision is the key-value store revision when the request was applied.
		int64 nRaftTem   = 0 ; // raft_term is the raft term when the request was applied.
	}ResponseHeader;

	typedef struct _RangeRequest
	{
		enum SortOrder {
			NONE    = 0, // default, no sorting
			ASCEND  = 1, // lowest target value first
			DESCEND = 2, // highest target value first
		};

		enum SortTarget {
			KEY     = 0,
			VERSION = 1,
			CREATE  = 2,
			MOD     = 3,
			VALUE   = 4,
		};
		Value sKey              ; // key is the first key for the range. If range_end is not given, the request only looks up key.
		Value sRangeEnd         ; // range_end is the upper bound on the requested range [key, range_end).
		bool  bPrefixed = false ; // if bPrefixed is true ,then the range request gets all keys prefixed with key.(模糊查询)
		bool  bEqOrGre  = false ; // if bEqOrGre is true, the range is all keys >= key.
		bool  bAllKey   = false ; // if bAllKey is true, then the rang request gets all keys.
		Value nLimit            ; // limit is a limit on the number of keys returned for the request. When limit is set to 0, it is treated as no limit.
		//revision is the point-in-time of the key-value store to use for the range.
		//if revision is less or equal to zero, the rang is over the newest key-value store.
		//if the revision has been compacted, ErrCompacted is returned as a response.
		Value nRevision         ; 
		Value eSortOrder        ; //must be the enum SortOrder type, set_order is the order for returned sorted results.
		Value eSortTarget       ; //must be the enum SortTarget type, sort_target is the key-value field to use for sorting.
		//serializable sets the range request to use serializable member-local reads.
		//Range request are linearizable by default; linearizable requests have higher
		//latency and lower throughput than serializable request but reflect the current 
		//consensus of cluster. For better performance, in exchange for possible stale reads,
		//a serializable range request is served locally without needing to reach consensus 
		//with other nodes in the cluster.
		Value bSerializeble    ;
		Value bKeyOnly         ; // keys_only when set returns only the keys and not the values.
		Value bCountOnly       ; // count_only when set returns only the count of the keys in the range.
		//min_mod_revision is the lower bound for returned key mod revisions; all keys with
		// lesser mod revisions will be filtered away.
		Value nMinModRevision  ;
		// max_mod_revision is the upper bound for returned key mod revisions; all keys with
		// greater mod revisions will be filtered away.
		Value nMaxModRevision  ;
		// min_create_revision is the lower bound for returned key create revisions; all
		// lesser create revisions will be filtered away.
		Value nMinCreateRevision;
		// max_create_revision is the upper bound for returned key create revisions; all
		// greater create revisions will be filtered away.
		Value nMaxCreateReviosn;
	}RangeRequest;


	typedef struct _RangeResponse
	{
		DefType<ResponseHeader> header         ;
		vector<KeyValue>        vKvs           ;  // kvs is the list of key-value pairs matched by the range request.kvs is empty when count is requested.
		bool                    bMore  = false ;  // more indicates if there are more keys to return in the requested range.
		int64                   nCount = 0     ;  // count is set to the number of keys within the range when requested.
	}RangeResponse;

	typedef struct _PutRequest
	{
		Value sKey   ; // key is the key, in bytes, to put into the key-value store.
		Value sValue ; // value is the value, in bytes, to associate with the key in the key-value store.
		 // lease is the lease ID to associate with the key in the key-value store. A lease
		 // value of 0 indicates no lease.
		Value nLease ;
		// If prev_kv is set, etcd gets the previous key-value pair before changing it.
		// The previous key-value pair will be returned in the put response.
		Value bPreKv;
		// If ignore_value is set, etcd updates the key using its current value.
		// Returns an error if the key does not exist.
		Value bIgnoreValue;
		// If ignore_lease is set, etcd updates the key using its current lease.
		// Returns an error if the key does not exist.
		Value bIgnoreLease;
	}PutRequest;

	typedef struct _PutResponse
	{
		DefType<ResponseHeader> header;
		DefType<KeyValue>       preKv;// if prev_kv is set in the request, the previous key-value pair will be returned.
	}PutResponse;

	typedef struct _DeleteRangeRequest
	{
		// key is the first key to delete in the range.
		Value sKey;
		// range_end is the key following the last key to delete for the range [key, range_end).
		// If range_end is not given, the range is defined to contain only the key argument.
		Value sRangeEnd;
		// 模糊删除if bPrefixe is true, then the range is all the keys with the prefix(the given key)
		bool  bPrefixe = false;
		// if the bEqualOrGreater is true, the rang is all keys greater than or equal to the key argument.
		bool  bEqualOrGreater = false;
		// if bAllKey is true, then the rang is all keys.
		bool  bAllKey= false;
		// If prev_kv is set, etcd gets the previous key-value pairs before deleting it.
		// The previous key-value pairs will be returned in the delete response.
		Value bPrevKv;
	}DeleteRangeRequest;

	typedef struct _DeleteRangeResponse
	{
		DefType<ResponseHeader> header;
		// deleted is the number of keys deleted by the delete range request.
		int64 nDeleted=0;
		// if prev_kv is set in the request, the previous key-value pairs will be returned.
		vector<KeyValue> vPreKvs;
	}DeleteRangeResponse;


	class  TxnRequest;
	class  TxnResponse;

	//only one type of them(RangeRequest,PutRequest,DeleteRangeRequest,TxnRequest) can be set .
	//can only get a non-nullptr/null pointer by call one of them(begin with the "Get" function,for example: GetsPtrRequestRang);
	//clear it by calling the void Clear() befor reset it ,
	class RequestOp
	{
	public:
		const std::shared_ptr<RangeRequest>&        GetsPtrRequestRang()              ;
		const std::shared_ptr<PutRequest>&          GetsPtrRequestPut()               ;
		const std::shared_ptr<DeleteRangeRequest>&  GetsPtrRequestDeleteRangRequest() ;
		const std::shared_ptr<TxnRequest>&          GetsPtrRequestTxn()               ;

		const std::shared_ptr<RangeRequest>&        GetConstsPtrRequestRang() const             ;
		const std::shared_ptr<PutRequest>&          GetConstsPtrRequestPut()const               ;
		const std::shared_ptr<DeleteRangeRequest>&  GetConstsPtrRequestDeleteRangRequest()const ;
		const std::shared_ptr<TxnRequest>&          GetConstsPtrRequestTxn()const               ;

		void Clear();

	private:

		bool Empty();

		std::shared_ptr<RangeRequest>        sPtrRequestRang              ;
		std::shared_ptr<PutRequest>          sPtrRequestPut               ;
		std::shared_ptr<DeleteRangeRequest>  sPtrRequestDeleteRangRequest ;
		std::shared_ptr<TxnRequest>          sPtrRequestTxn               ;
	};

	class ResponseOp
	{
	public:

		const std::shared_ptr<RangeResponse>&       GetsPtrResponseRang()        ;
		const std::shared_ptr<PutResponse>&         GetsPtrResponsePut()         ;
		const std::shared_ptr<DeleteRangeResponse>& GetsPtrResponseDeleteRange() ;
		const std::shared_ptr<TxnResponse>&         GetsPtrResponseTxn()         ;

		void Clear();

	private:

		bool Empty();

		std::shared_ptr<RangeResponse>       sPtrResponseRang        ;
		std::shared_ptr<PutResponse>         sPtrResponsePut         ;
		std::shared_ptr<DeleteRangeResponse> sPtrResponseDeleteRange ;
		std::shared_ptr<TxnResponse>         sPtrResponseTxn         ;
	};

	typedef struct _Compare
	{
		enum CompareResult {
			EQUAL     = 0,
			GREATER   = 1,
			LESS      = 2,
			NOT_EQUAL = 3,
		};
		enum CompareTarget {
			VERSION = 0,
		    CREATE  = 1,
		    MOD     = 2,
		    VALUE   = 3,
		};

		//only one value of member variables can be set.
		//clear it by calling member function void Clear() before reset it.
		class TargetUnion
		{
		public:

			bool SetnVersion(int64 nVersion)               ;
			bool SetnCreateRevision(int64 nCreateRevision) ;
			bool SetnModRevision(int64 nModRevision)       ;
			bool SetsValue(const string& sValue)           ;

			bool GetnVersion(int64& nVersion) const               ;
			bool GetnCreateRevision(int64& nCreateRevision) const ;
			bool GetnModRevision(int64& nModRevision) const       ;
			bool GetsValue(string& sValue)  const                 ;

			void Clear();

			
		private:

			bool Empty();

			Value m_nVersion        ; // the version of the given key
			Value m_nCreateRevision ; // the creation revision of the given key
			Value m_nModRevision    ; // the last modified revision of the given key 
			Value m_sValue          ; // the value of the given key, in bytes.
		};

		
		Value nResult ; // result is logical comparison operation for this comparison.
		Value nTarget ; // target is the key-value field to inspect for the comparison.
		Value sKey    ; // key is the subject key for the comparison operation.
		TargetUnion targetUnion;
		Value sRangeEnd; // range_end compares the given target to all keys int the range [key, range_end).

	}Compare;



	class TxnRequest
	{
	public:

		//vCompare is a list of predicates representing conjunction of terms.
		//if the comparisons succeed, then the vSuccess requests will be processed in order.
		// and the comparisons fail, then the vFailure request will be processed in order.
		// and the response will contain their respectively response in order.
		// notice: if the vCompare is empty, the  vSuccess request will be processed in order 
		// and the bSucesseded in TxnResponse will return true
		std::vector<Compare>   vCompare;
		// vSuccess is a list of requests which will be applied when the compare evaluates to true.
		std::vector<RequestOp> vSuccess;
		//vFailure is a list of requests which will be applied when the compare evaluates to false.
		std::vector<RequestOp> vFailure;
	};

	class TxnResponse
	{
	public:
		DefType<ResponseHeader> header     ;
		// bSucesseded is set to true if the compare evaluated to true or false otherwise.
		bool bSucesseded      = false      ;
		// vResponses is a list of response corresponding to results from applying
		// success if bSucesseded is true or failure if bSucesseded  is false.
		vector<ResponseOp>      vResponses ;
	};

	// CompactionRequest compacts the key-valaue store up to given revision. All superseded keys 
	//with a revision less than the compaction revision will be removed.
	typedef struct _CompactionRequest
	{
		//revision is the key-value store revision for the compaction operation.
		Value nRevision;
		//physical is set so the RPC will wait until the compaction is physically applied to the 
		// local database such that compacted entries are totally remove from the backend database.
		Value bPhysical;

	}CompactionRequest;

	typedef struct _CompactionResponse
	{
		DefType<ResponseHeader> header;
	}CompactionResponse;


	typedef struct _WatchCreateRequest
	{
		// key is the key to register for watching 
		Value sKey;
		// rang_end is the end of the range [key, range_end) to watch. If range_end is not given, only
		//the key argument is watched .
		Value sRangeEnd;
		// if the bAllKey is true,then the rang request watch the all keys.
		bool  bAllKey  = false;
		// if bEqOrGre is true ,then  all keys greater than or equal to the key argument are watched.
		bool  bEqOrGre = false;
		// if bPrefixed is true ,then all keys with prefix(the given key) will be watched.
		bool  bPrefixed = false;
		// nStartRevision is and optional revision to watch from (inclusive).No nStartRevision is "now".
		Value nStartRevision;
		//bProgressNotify is set so that the etcd server will periodically send a WatchResponse whith
		//no events to the new watcher if there are no recent events .It is useful when clients wish
		//to recover a disconnected watcher starting from a recent known revision. the etcd server
		//may decided how often it will send notification based on current load.
		Value bProgressNotify;

		enum FilterType {
			// filter out put event.
			NOPUT = 0,
			// filter out delete event.
			NODELETE = 1,
		};

		//vFilters filter the events at server side before it sends back to the watcher.
		vector<FilterType> vFilters;

		// if bPreKv is set, created watcher gets the previous KV before the event happens.
		// if the previous KV is already compacted. nothing will be returned.
		Value bPreKv;

	}WatchCreateRequest;

	typedef struct _WatchCancelRequest
	{
		//nWatchId is the watcher id to cancel so that no more events are transmitted.
		Value nWatchId;
	}WatchCancelRequest;


	class WatchRequest
	{
	public:

		const shared_ptr<WatchCreateRequest>& GetsPtrWatchCreateRequest();
		const shared_ptr<WatchCancelRequest>& GetsPtrWatchCancelRequest();

		const shared_ptr<WatchCreateRequest>& GetcsPtrWatchCreateRequest() const;
		const shared_ptr<WatchCancelRequest>& GetcsPtrWatchCancelRequest() const;


		void Clear();
		bool Empty() const;

	private:

		shared_ptr<WatchCreateRequest> sPtrWatchCreateRequest;
		shared_ptr<WatchCancelRequest> sPtrWatchCancelRequest;
	};

	typedef struct _WatchResponse
	{
		DefType<ResponseHeader> header;
		//nWatchId is the ID of the watcher that corresponds to the response.
		int64 nWatchId = 0;
		//created is set to true  if the response is for a create watch request.
		//The client should record the nWatchId and expect to received events for 
		//the created watcher form the same stream. All events set to the created 
		//watched will attch with the same nWatchId.
		bool bCreated = false;
		// canceled is set to true if the response is for a cancel watch request.
		//no further events will be sent to the canceled watcher.
		bool bCanceled = false;
		//nCompactRevision is set to the minimum index if a watcher tries to watch at 
		// a compacted index. 

		//This happens when creating a watcher at a compacted revision or the 
		//watcher cannot catch up with the progress of the the key -value store.

		// the client should treat that watcher as canceled and should not try to 
		//create any watcher with the progress of the key-value store.
		int64 nCompactRevision = 0;
		// sCancelReson indicates the reason for canceling the watcher.
		string sCancelReson;
		vector<Event> vEvents;
	}WatchResponse;


	typedef struct _LeaseGrantRequest
	{
		// nTTL is the advisory time-to-live in seconds.
		Value nTTL; 
		// ID is the requested ID for the lease. If ID is set to 0, the lessor chooses an ID.
		Value nID;
	}LeaseGrantRequest;

	typedef struct  _LeaseGrantResponse
	{
		DefType<ResponseHeader> header;
		// nID is the lease ID for the granted lease.
		int64 nID = 0;
		// TTL is the server chosen lease time-to-live in seconds.
		int64 nTTL = 0;
		string sError;
	}LeaseGrantResponse;

	typedef struct _LeaseRevokeRequest
	{
		// ID is the lease ID to revoke. When the ID is revoked, all associated keys will be deleted.
		Value nID;
	}LeaseRevokeRequest;

	typedef struct _LeaseRevokeResponse
	{
		DefType<ResponseHeader> header;
	}LeaseRevokeResponse;

	typedef struct _LeaseKeepAliveRequest
	{
		// nID is the lease ID for the lease to keep alive. 
		Value nID;
	}LeaseKeepAliveRequest;

	typedef struct _LeaseKeepAliveResponse
	{
		DefType<ResponseHeader> header;
		// nID is the lease ID from the keep alive request
		int64 nID=0;
		// TTL is the new time-to-alive for the lease.
		int64 nTTL=0;
	}LeaseKeepAliveResponse;

	typedef struct _LeaseTimeToLiveRequest
	{
		//nID is the lease ID for the lease.
		Value nID;
		//bKeys is true to query all the keys attached to this lease.
		Value bKeys;
	}LeaseTimeToLiveRequest;

	typedef struct _LeaseTimeToLiveResponse
	{
		DefType<ResponseHeader> header;
		// nID is the lease ID form the keep alive request
		int64 nID=0;
		// TTL is the remaining TTL in seconds for the lease; the lease will expired in under TTL+1 seconds.
		int64 nTTL=0;
		//nGrantedTTL is the initial granted time in seconds upon lease creation/renewal.
		int64 nGrantedTTL=0;
		//vKeys is the list of keys attached to the lease.
		vector<string> vKeys;
	}LeaseTimeToLiveResponse;

	typedef struct _AuthEnableRequest
	{

	}AuthEnableRequest;

	typedef struct _AuthEnableResponse
	{
		DefType<ResponseHeader> header;
	}AuthEnableResponse;

	typedef struct _AuthDisableRequest
	{

	}AuthDisableRequest;

	typedef struct _AuthDisableResponse
	{
		DefType<ResponseHeader> header;
	}AuthDisableResponse;

	typedef struct _AuthenticateRequest
	{
		Value sName;
		Value sPassWord;
	}AuthenticateRequest;

	typedef struct _AuthenticateResponse
	{
		DefType<ResponseHeader> header;
		//sToken is an authorized token that can be used in succeding RPCs
		string sToken;
	}AuthenticateResponse;

	typedef struct _AuthUserAddRequest
	{
		Value sName;
		Value sPassWord;
	}AuthUserAddRequest;

	typedef struct _AuthUserAddResponse
	{
		DefType<ResponseHeader> header;
	}AuthUserAddResponse;

	typedef struct _AuthUserGetRequest
	{
		Value sName;
	}AuthUserGetRequest;

	typedef struct _AuthUserGetResponse
	{
		DefType<ResponseHeader> header;
		vector<string> vRoles;
	}AuthUserGetResponse;

	typedef struct _AuthUserListRequest
	{

	}AuthUserListRequest;

	typedef struct _AuthUserListResponse
	{
		DefType<ResponseHeader> header;
		vector<string> vUsers;
	}AuthUserListResponse;

	typedef struct _AuthUserDeleteRequest
	{
		Value sName;
	}AuthUserDeleteRequest;

	typedef struct _AuthUserDeleteResponse
	{
		DefType<ResponseHeader> header;
	}AuthUserDeleteResponse;

	typedef struct _AuthUserChangePasswordRequest
	{
		Value sName;
		Value sPassWord;
	}AuthUserChangePasswordRequest;

	typedef struct _AuthUserChangePasswordResponse
	{
		DefType<ResponseHeader> header;
	}AuthUserChangePasswordResponse;

	typedef struct _AuthUserGrantRoleRequest
	{
		Value sUser;
		Value sRole;
	}AuthUserGrantRoleRequest;

	typedef struct _AuthUserGrantRoleResponse
	{
		DefType<ResponseHeader> header;
	}AuthUserGrantRoleResponse;

	typedef struct _AuthUserRevokeRoleRequest
	{
		Value sName;
		Value sRole;
	}AuthUserRevokeRoleRequest  ;

	typedef struct _AuthUserRevokeRoleResponse
	{
		DefType<ResponseHeader> header;
	}AuthUserRevokeRoleResponse;

	typedef struct _AuthRoleAddRequest
	{
		Value sName;
	}AuthRoleAddRequest;

	typedef struct _AuthRoleAddResponse
	{
		DefType<ResponseHeader> header;
	}AuthRoleAddResponse;

	typedef struct _AuthRoleGetRequest
	{
		Value sRole;
	}AuthRoleGetRequest;

	typedef struct _AuthRoleGetResponse
	{
		DefType<ResponseHeader> header;
		vector<Permission> vPerm;

	}AuthRoleGetResponse;

	typedef struct _AuthRoleListRequest
	{

	}AuthRoleListRequest;

	typedef struct _AuthRoleListResponse
	{
		DefType<ResponseHeader> header;
		vector<string> vRoles;
	}AuthRoleListResponse;

	typedef struct _AuthRoleDeleteRequest
	{
		Value sRole;
	}AuthRoleDeleteRequest;

	typedef struct _AuthRoleDeleteResponse
	{
		DefType<ResponseHeader> header;
	}AuthRoleDeleteResponse;

	typedef struct _AuthRoleGrantPermissionRequest
	{
		Value sName;
		PermissionValue permValue;
	}AuthRoleGrantPermissionRequest;

	typedef struct _AuthRoleGrantPermissionResponse
	{
		DefType<ResponseHeader> header;

	}AuthRoleGrantPermissionResponse;

	typedef struct _AuthRoleRevokePermissionRequest
	{
		Value sRole;
		Value sKey;
		Value sRangeEnd;
	}AuthRoleRevokePermissionRequest;

	typedef struct _AuthRoleRevokePermissionResponse
	{
		DefType<ResponseHeader> header;
	}AuthRoleRevokePermissionResponse;
}