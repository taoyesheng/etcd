//2018-10-15
//tys create
// etcd client type realization
#include "EtcdClientType.h"

namespace etcdclient
{

	Value::Value() :m_type(0), m_len(0)
	{

	}

	Value::Value(bool b) : m_type(1), m_len(sizeof(bool))
	{
		m_data.m_bool = b;
	}

	Value::Value(int n) : m_type(2), m_len(sizeof(long long))
	{
		m_data.m_int = n;
	}

	Value::Value(unsigned int n) : m_type(2), m_len(sizeof(long long))
	{
		m_data.m_int = n;
	}

	Value::Value(long long n) : m_type(2), m_len(sizeof(long long))
	{
		m_data.m_int = n;
	}

	Value::Value(float f) : m_type(3), m_len(sizeof(double))
	{
		m_data.m_float = f;
	}

	Value::Value(double d) : m_type(3), m_len(sizeof(double))
	{
		m_data.m_float = d;
	}

	Value::Value(const string& s) : m_type(4), m_len(s.length())
	{
		m_data.m_ptr = malloc(m_len + 1);
		memcpy(m_data.m_ptr, s.c_str(), m_len + 1);
	}

	Value::Value(const char* s) : m_type(4), m_len(0)
	{
		assert(s);
		m_len = strlen(s);

		m_data.m_ptr = malloc(m_len + 1);
		memcpy(m_data.m_ptr, s, m_len + 1);
	}

	Value::Value(const void* ptr, size_t len) :m_type(5), m_len(len)
	{
		assert(ptr);
		assert(len > 0);
		m_data.m_ptr = malloc(len + 1);
		memcpy(m_data.m_ptr, ptr, len);
	}

	Value::Value(const Value& hs) :m_type(hs.m_type), m_len(hs.m_len)
	{
		switch (m_type)
		{
		case 1:
			m_data.m_bool = hs.m_data.m_bool;
			break;
		case 2:
			m_data.m_int = hs.m_data.m_int;
			break;
		case 3:
			m_data.m_float = hs.m_data.m_float;
			break;
		case 4:
			m_data.m_ptr = malloc(m_len + 1);
			memcpy(m_data.m_ptr, hs.m_data.m_ptr, m_len + 1);
			break;
		case 5:
			m_data.m_ptr = malloc(m_len + 1);
			memcpy(m_data.m_ptr, hs.m_data.m_ptr, m_len);
			break;
		case 6:
			m_data.m_time = hs.m_data.m_time;
			break;
		case 0:
		default:
			m_type = 0;
			m_len = 0;
			break;
		}
	}

	Value::Value(Value&& hs) :Value()
	{
		std::swap(m_type, hs.m_type);
		std::swap(m_len, hs.m_len);
		std::swap(m_data, hs.m_data);
	}

	Value& Value::operator =(const Value& hs)
	{
		if (this == &hs)
			return *this;

		if (4 == m_type || 5 == m_type)
		{
			free(m_data.m_ptr);
			m_data.m_ptr = nullptr;
		}
		m_type = hs.m_type;
		m_len = hs.m_len;


		switch (m_type)
		{
		case 1:
			m_data.m_bool = hs.m_data.m_bool;
			break;
		case 2:
			m_data.m_int = hs.m_data.m_int;
			break;
		case 3:
			m_data.m_float = hs.m_data.m_float;
			break;
		case 4:
			m_data.m_ptr = malloc(m_len + 1);
			memcpy(m_data.m_ptr, hs.m_data.m_ptr, m_len + 1);
			break;
		case 5:
			m_data.m_ptr = malloc(m_len + 1);
			memcpy(m_data.m_ptr, hs.m_data.m_ptr, m_len);
			break;
		case 6:
			m_data.m_time = hs.m_data.m_time;
			break;
		case 0:
		default:
			m_type = 0;
			m_len = 0;
			break;
		}

		return *this;
	}

	Value& Value::operator = (Value&& hs)
	{
		if (this == &hs)
			return *this;

		if (4 == m_type || 5 == m_type)
		{
			free(m_data.m_ptr);
			m_data.m_ptr = nullptr;
		}
		m_type = 0;
		m_len  = 0;

		std::swap(m_type, hs.m_type);
		std::swap(m_len, hs.m_len);
		std::swap(m_data, hs.m_data);

		return *this;
	}

	Value::~Value()
	{
		if (4 == m_type || 5 == m_type)
		{
			free(m_data.m_ptr);
			m_data.m_ptr = nullptr;
		}

		m_type = 0;
		m_len = 0;
	}

	string Value::ToString() const
	{
		string sRetStr;

		switch (m_type)
		{
		case 1:
			sRetStr = m_data.m_bool ? "true" : "false";
			break;
		case 2:
			sRetStr = std::to_string(m_data.m_int);
			break;
		case 3:
			sRetStr = std::to_string(m_data.m_float);
			break;
		case 4:
		case 5:
			sRetStr.append((char*)m_data.m_ptr, m_len);
			break;
		case 6:
			sRetStr = std::to_string(m_data.m_time);
			break;
		case 0:
		default:
			break;
		}

		return ::move(sRetStr);
	}

	bool Value::Empty() const
	{
		return 0 == m_type;
	}

	bool Value::GetBool()const
	{
		assert(1 == m_type);
		return m_data.m_bool;
	}

	long long Value::GetInt()const
	{
		assert(2 == m_type);
		return m_data.m_int;
	}

	double Value::GetDouble() const
	{
		assert(3 == m_type);
		return m_data.m_float;
	}

	string Value::GetString() const
	{
		assert(4 == m_type);
		return ::move(ToString());
	}

	string Value::GetBinary() const
	{
		assert(5 == m_type);
		return ::move(ToString());
	}

	const std::shared_ptr<etcdclient::RangeRequest>& RequestOp::GetsPtrRequestRang()
	{
		if (Empty())
			sPtrRequestRang = std::make_shared<etcdclient::RangeRequest>();

		return sPtrRequestRang;
	}

	const std::shared_ptr<etcdclient::PutRequest>& RequestOp::GetsPtrRequestPut()
	{
		if (Empty())
			sPtrRequestPut = std::make_shared<etcdclient::PutRequest>();

		return sPtrRequestPut;
	}

	const std::shared_ptr<etcdclient::DeleteRangeRequest>& RequestOp::GetsPtrRequestDeleteRangRequest()
	{
		if (Empty())
			sPtrRequestDeleteRangRequest = std::make_shared<etcdclient::DeleteRangeRequest>();

		return sPtrRequestDeleteRangRequest;
	}

	const std::shared_ptr<etcdclient::TxnRequest>& RequestOp::GetsPtrRequestTxn()
	{
		if (Empty())
			sPtrRequestTxn = std::make_shared<etcdclient::TxnRequest>();

		return sPtrRequestTxn;
	}


	const std::shared_ptr<etcdclient::RangeRequest>& RequestOp::GetConstsPtrRequestRang() const
	{
		return sPtrRequestRang;
	}

	const std::shared_ptr<etcdclient::PutRequest>& RequestOp::GetConstsPtrRequestPut() const
	{
		return sPtrRequestPut;
	}

	const std::shared_ptr<etcdclient::DeleteRangeRequest>& RequestOp::GetConstsPtrRequestDeleteRangRequest() const
	{
		return sPtrRequestDeleteRangRequest;
	}

	const std::shared_ptr<etcdclient::TxnRequest>& RequestOp::GetConstsPtrRequestTxn() const
	{
		return sPtrRequestTxn;
	}

	void RequestOp::Clear()
	{
		sPtrRequestRang = nullptr;
		sPtrRequestPut = nullptr;
		sPtrRequestDeleteRangRequest = nullptr;
		sPtrRequestTxn = nullptr;
	}

	bool RequestOp::Empty()
	{
		return nullptr == sPtrRequestPut && nullptr == sPtrRequestRang &&
			nullptr == sPtrRequestDeleteRangRequest && nullptr == sPtrRequestTxn;
	}

	const std::shared_ptr<etcdclient::RangeResponse>& ResponseOp::GetsPtrResponseRang()
	{
		if (Empty())
			sPtrResponseRang = shared_ptr<etcdclient::RangeResponse>();

		return sPtrResponseRang;
	}

	const std::shared_ptr<etcdclient::PutResponse>& ResponseOp::GetsPtrResponsePut()
	{
		if (Empty())
			sPtrResponsePut = std::shared_ptr<etcdclient::PutResponse>();

		return sPtrResponsePut;
	}

	const std::shared_ptr<etcdclient::DeleteRangeResponse>& ResponseOp::GetsPtrResponseDeleteRange()
	{
		if (Empty())
			sPtrResponseDeleteRange = std::make_shared<DeleteRangeResponse>();

		return sPtrResponseDeleteRange;
	}

	const std::shared_ptr<etcdclient::TxnResponse>& ResponseOp::GetsPtrResponseTxn()
	{
		if (Empty())
			sPtrResponseTxn = std::make_shared<TxnResponse>();

		return sPtrResponseTxn;
	}

	void ResponseOp::Clear()
	{
		sPtrResponseRang        = nullptr ;
		sPtrResponsePut         = nullptr ;
		sPtrResponseDeleteRange = nullptr ;
		sPtrResponseTxn         = nullptr ;
	}

	bool ResponseOp::Empty()
	{
		return  nullptr == sPtrResponseRang && nullptr == sPtrResponsePut &&
			    nullptr == sPtrResponseDeleteRange && nullptr == sPtrResponseTxn;
	}

	bool _Compare::TargetUnion::SetnVersion(int64 nVersion)
	{
		if (!Empty())
			return false;

		this->m_nVersion = nVersion;

		return true;
	}

	bool _Compare::TargetUnion::SetnCreateRevision(int64 nCreateRevision)
	{
		if (!Empty())
			return false;

		this->m_nCreateRevision = nCreateRevision;

		return true;
	}

	bool _Compare::TargetUnion::SetnModRevision(int64 nModRevision)
	{
		if (!Empty())
			return false;

		this->m_nModRevision = nModRevision;

		return true;
	}

	bool _Compare::TargetUnion::SetsValue(const string& sValue)
	{
		if (!Empty())
			return false;

		this->m_sValue = sValue;

		return true;
	}

	void _Compare::TargetUnion::Clear()
	{
		if (Empty())
			return;

		Value empty;

		if (!m_nVersion.Empty())
			m_nVersion = empty;
		
		if (!m_nCreateRevision.Empty())
			m_nCreateRevision = empty;

		if (!m_nModRevision.Empty())
			m_nModRevision = empty;

		if (!m_sValue.Empty())
			m_sValue = empty;
	}

	bool _Compare::TargetUnion::GetnVersion(int64& nVersion) const
	{
		if (this->m_nVersion.Empty())
			return false;

		 nVersion = this->m_nVersion.GetInt();

		return true;
	}

	bool _Compare::TargetUnion::GetnCreateRevision(int64& nCreateRevision) const
	{
		if (this->m_nCreateRevision.Empty())
			return false;

		 nCreateRevision = this->m_nCreateRevision.GetInt();

		return true;
	}

	bool _Compare::TargetUnion::GetnModRevision(int64& nModRevision) const
	{
		if (this->m_nModRevision.Empty())
			return false;

		 nModRevision = this->m_nModRevision.GetInt();

		return true;
	}

	bool _Compare::TargetUnion::GetsValue(string& sValue) const
	{
		if (this->m_sValue.Empty())
			return false;

		sValue = ::move(this->m_sValue.GetString());

		return true;
	}

	bool _Compare::TargetUnion::Empty()
	{
		return m_nVersion.Empty() && m_nCreateRevision.Empty()&&m_nModRevision.Empty()&&m_sValue.Empty();
	}

	const std::shared_ptr<etcdclient::WatchCreateRequest>& WatchRequest::GetsPtrWatchCreateRequest()
	{
		if (Empty())
			sPtrWatchCreateRequest = make_shared<WatchCreateRequest>();

		return sPtrWatchCreateRequest;
	}

	const std::shared_ptr<etcdclient::WatchCreateRequest>& WatchRequest::GetcsPtrWatchCreateRequest() const
	{
		return sPtrWatchCreateRequest;
	}

	const std::shared_ptr<etcdclient::WatchCancelRequest>& WatchRequest::GetsPtrWatchCancelRequest()
	{
		if (Empty())
			sPtrWatchCancelRequest = make_shared<WatchCancelRequest>();

		return sPtrWatchCancelRequest;
	}

	const std::shared_ptr<etcdclient::WatchCancelRequest>& WatchRequest::GetcsPtrWatchCancelRequest() const
	{
		return sPtrWatchCancelRequest;
	}

	void WatchRequest::Clear()
	{
		sPtrWatchCancelRequest = nullptr;
		sPtrWatchCreateRequest = nullptr;
	}

	bool WatchRequest::Empty() const
	{
		return nullptr == sPtrWatchCancelRequest && nullptr == sPtrWatchCreateRequest;
	}

}