#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <fstream>
#include <sstream>

#include <grpcpp/grpcpp.h>


#include "lock.grpc.pb.h"
#include "rpc.grpc.pb.h"

#include "EtcdClient.h"
//#include "vld.h"




using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;

using namespace  std;

using namespace etcdclient;


int main(int argc, char* argv[])
{
	string sIpAndPort = "10.0.14.78:2399";
	//if (argc >= 2)
	//	sIpAndPort = argv[1];
	//else
	//{
	//	cout << "please input ip and port " << endl;
	//	cin >> sIpAndPort;
	//}

	std::shared_ptr<etcdclient::EtcdClient> sPtrEtcdClient = std::make_shared<etcdclient::EtcdClient>();

	while (true)
	{
		sPtrEtcdClient->InitKV(sIpAndPort);

		etcdclient::RangeRequest   reqRange;
		etcdclient::RangeResponse  resRange;

		//第一次模糊查询 前缀为 "test" 的全部数据
		reqRange.sKey = "test";
		reqRange.bPrefixed = true;
		int nRet = -1;

		 nRet = sPtrEtcdClient->Range(reqRange, resRange);
		if (E_G_VALUE::SUCCESS != nRet)
			continue;


		//用watch模糊监听前缀为"tys"的增量数据
		etcdclient::WatchRequest   reqWatch;
		etcdclient::WatchResponse  resWatch;

		sPtrEtcdClient->InitWatch(sIpAndPort);
		nRet = sPtrEtcdClient->Watch();
		if (E_G_VALUE::SUCCESS != nRet)
			continue;
		sPtrEtcdClient->SetbStore(true);

		auto pCreate = reqWatch.GetsPtrWatchCreateRequest();
		pCreate->sKey = "tys";
		pCreate->bPrefixed = true;

		nRet = sPtrEtcdClient->Write(reqWatch);
		if (E_G_VALUE::SUCCESS != nRet)
			continue;

		while (sPtrEtcdClient->GetbWatchNormal())
		{
			std::shared_ptr<WatchResponse> ptrRes;
			sPtrEtcdClient->TakeResponse(ptrRes);
			if (nullptr != ptrRes)
			{
				cout << "tys test Watch " << endl;
			}

			this_thread::sleep_for(chrono::microseconds(100));
		}

		static int nTimes = 1;
		cout << nTimes++ << " tttttt " << endl;

		this_thread::sleep_for(chrono::seconds(10));
	}

	return 0;
}
