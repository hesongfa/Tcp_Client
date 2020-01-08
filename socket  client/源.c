#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#include <Ws2tcpip.h>
//#include <arpe/inet.h>
//#define SERVER_PORT 9210 //侦听端口

#define SERVER_PORT 1025 //侦听端口
#pragma comment(lib,"ws2_32.lib")


//数据包
struct Data
{
	unsigned char sendMessage[1026];
	int fin;
}data;


/*这些应该都是之前留下的东西*/
//typedef struct Point2D64f
//{
//	double x;
//	double y;
//}Point2D64f;
//
//typedef struct feature
//{
//	double x;      /**< x coord */ //特征点的x坐标
//	double y;      /**< y coord *///特征点的y坐标
//	double a;      //OXFD特征点中椭圆的参数       /**< Oxford-type affine region parameter */
//	double b;      //OXFD特征点中椭圆的参数       /**< Oxford-type affine region parameter */
//	double c;      //OXFD特征点中椭圆的参数       /**< Oxford-type affine region parameter */
//	double scl;    //LOWE特征点的尺度             /**< scale of a Lowe-style feature */
//	double ori;    //LOWE特征点的方向             /**< orientation of a Lowe-style feature */
//	int d;         //特征描述子的长度，即维数，一般是128       /**< descriptor length */
//	double descr[128];   //128维的特征描述子，即一个double数组  /**< descriptor */
//	int type;      //特征点类型                  /**< feature type, OXFD or LOWE */
//	int category;                  /**< all-purpose feature category */
//	struct feature* fwd_match; //指明此特征点对应的匹配点  /**< matching feature from forward image */
//	struct feature* bck_match; //指明此特征点对应的匹配点  /**< matching feature from backmward image */
//	struct feature* mdl_match; //指明此特征点对应的匹配点  /**< matching feature from model */
//	Point2D64f img_pt;          //特征点的坐标,等于(x,y)       /**< location in image */
//	Point2D64f mdl_pt;          //当匹配类型是mdl_match时用到   /**< location in model */
//	void* feature_data;        //用户定义的数据:    /**< user-definable data */
//}Feature;


void main()
{
	struct Data data;
	FILE * fp;
	struct feature* feat1;   //这个就没用 hsf
	char title[] = "BM1 \b \0 WO AI NI 1 2 3 4 5 6 7 8 9 10 11 12 13";  //这个也没用 hsf
	
	unsigned char info[1024];    //开辟出来存放图像本身字节的空间
	unsigned char infosend[1024];

	unsigned char info14[14];   //这个空间开辟出来存放位图文件头的字节
	unsigned char info40[40];   //                 存放位图信息头的字节 hsf
	
	errno_t err;  //判断此文件流是否存在 存在返回0
 	err = fopen_s(&fp, "F://591.bmp", "rb+"); //若return 1 , 则将指向这个文件的文件流给fp1
	//fp = fopen("E://example.bmp", "rb+");
	if (err != 0)
	{
		printf("open F://591.bmp fail!");
		return ;
	}

	WORD wVersionRequested;  //2个字节 hsf
	WSADATA wsaData;		 //这个结构被用来存储被WSAStartup函数调用后返回的Windows Sockets数据。
						    //它包含Winsock.dll执行的数据。它放在这里应该是没用的
	
	int ret; //套接字状态
	int III = 0;

	SOCKET sClient;					//连接套接字
	struct sockaddr_in saServer;	//地址信息
	char *ptr;
	BOOL fSuccess = TRUE;

	fseek(fp, 0, SEEK_END);		//这里光标定位到结尾处	hsf
	int end = ftell(fp);		//这里就算出了文件的总大小（字节）hsf
	fseek(fp, 0, 0);			//这里又定位回来文件头处  hsf
	
	//WinSock初始化
	wVersionRequested = MAKEWORD(2, 2); //希望使用的WinSock DLL的版本
	ret = WSAStartup(wVersionRequested, &wsaData);   //返回套接字状态hsf
	
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return;
	} 
	//确认WinSock DLL支持版本2.2
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid WinSock version!\n");
		return;
	}
	//创建Socket,使用TCP协议
	sClient = socket(AF_INET, SOCK_STREAM, 0);
	if (sClient == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return;
	}
	//构建服务器地址信息
	saServer.sin_family = AF_INET; //地址家族
	saServer.sin_port = htons(SERVER_PORT); //注意转化为网络节序
	//saServer.sin_addr.S_un.S_addr = inet_addr("192.168.1.111");

	//此处地址是DSP端作为服务器的ip地址
	inet_pton(AF_INET, "192.168.0.118", &saServer.sin_addr.S_un.S_addr);   //  代替 foo.sin_addr.addr=inet_addr(ip);
	//连接服务器
	ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("connect() failed!\n");
		closesocket(sClient); //关闭套接字
		WSACleanup();
		return;
	}
	char sendMessage[2000];     //定义了一块内存 hsf
	int sum = 0;
	int times = 0;
	char flag = '0';

	int shuchu;

	memset(info14, '0', sizeof(info14));  //将info14这一块内存的内容全部换成字符‘0’hsf
	fread(info14, 14, 1, fp);             //从文件fp中读取14个字符到内存块info14中 ，读取完了之后，
										 // 此时光标已经移到文件的第15个字节前了hsf
	ret = send(sClient, (unsigned char *)&info14, 14, 0);   //从info14发送14个字符到DSP端，就是文件头信息 hsf
	end = end - 14;     //减掉位图文件头信息字节数后，剩余的字节数

	for (shuchu = 0; shuchu < 14; shuchu++)
	{
		printf("%d\n", info14[shuchu]);    //这里是可以查看到位图文件头的每一个字节
	}

	memset(info40, '0', sizeof(info40));   //  内存初始化，置字符‘0’
	fread(info40, 40, 1, fp);			   //  从文件fp中读取40个字符到内存块info40中 ，读取完了之后，
										   //  此时光标已经移到文件的第54个字节前了hsf
	ret = send(sClient, (unsigned char *)&info40, 40, 0);   //从info14发送40个字符到DSP端，就是位图信息头字节 hsf
	end = end - 40;						//再减掉位图信息头的字节数后，剩余的字节数就是图像数据本身的字节数了

	unsigned char *pBufn;
	int nr;
	struct Data datan;
	int cishu=0;

	int ic = 0;

	int receive[2];
	int re;

	int jishu = 0;   //我用来统计发出多少字节
	int total_zijix = 0;  //我用来计算总共发出多少字节

 	while (end>0)      //用剩余字节数的方法退出循环发送
	{
		memset(info, '0', sizeof(info));   //初始化info空间
		fread(info, 1024, 1, fp);		  //  从文件fp中读取字符到内存块info中 ，读取完了之后，
										  //  此时光标已经移到文件的第54+n个字节前了hsf 

		jishu++;
		printf("sending.....%d\n",jishu);         //我用这句代替,起标识作用,位置应该放这里才对


		if (end >= 1024) //剩余很多信息
		{
			III = 1024;
		}
		else
		{
			III = end;	
			total_zijix = ((jishu-1) * 1024) + III;
			printf("图像传输完成!!! 总字节数：%d\n",total_zijix);
		}
		//这一段起将最后不足一块1024字节的数据数目统计出来的作用

		int II = 8000000;//发送34M图像时II=400000;发送1.4M图像时II=4000;发送2.14M图像用II=800;
		while (II > 0)//II=8000000时，可以发送2000*1500的8Mb的图像
		{
			II = II - 1;
		}
		//这段函数应该是放在这里起缓冲的作用，就是那位大哥所说的降低发送速度 hsf

		ret = send(sClient, (unsigned char *)info, III, 0);	   //向DSP端发送 III 个字节的图像数据
															   //如果无错误，返回值为所发送数据的总数，
															  // 否则返回SOCKET_ERROR。
		//客户程序一般用send函数向服务器发送请求，而服务器则通常用send函数来向客户程序发送应答。

		//printf("%d\n", receive[0]);     //这一句就没用了，而且我觉得他位置放错了 hsf
		//printf("sending.....\n");         //我用这句代替,起标识作用

		if (ret == SOCKET_ERROR)
		{
			printf("send() failed!\n");
		}

		else		
  			end -= 1024;   //每发送一次减一次
	}

	closesocket(sClient); //关闭套接字
	WSACleanup();
}









/*int W1 = (int)(info40[4]) + (int)(info40[5]) * 256 + (int)(info40[6]) * 256 * 256 + (int)(info40[7]) * 256 * 256 * 256;
int H1 = (int)(info40[8]) + (int)(info40[9]) * 256 + (int)(info40[10]) * 256 * 256 + (int)(info40[11]) * 256 * 256 * 256;
for (shuchu = 0; shuchu <40; shuchu++)
{
printf("%d\n", info40[shuchu]);
}
int end1 = end;
int shuchu1;
unsigned char *infosendto= (unsigned char*)malloc(W1 * H1 * 3 + 1024);
unsigned char infoto[1];
while (end1 > 0)
{
fread(infoto, 1, 1, fp);
infosendto[end1 - 1]=infoto[0];
end1 = end1 - 1;
}
shuchu1 = 0;
while (end>0)
{
memset(info, '0', sizeof(info));
strncpy_s(info,1024,infosendto+shuchu1, 1024);

if (end >= 1024) //剩余很多信息
{
III = 1024;
}
else
{
III = end;
}
int II = 800000;//发送34M图像时II=400000;发送1.4M图像时II=4000;发送2.14M图像用II=800;
while (II > 0)
{
II = II - 1;
}
//ret = send(sClient, (unsigned char *)&info, III, 0);
ret = send(sClient, (unsigned char *)&info, III, 0);//20191008新加
printf("%c\n", info);
printf("%c\n", info);
if (ret == SOCKET_ERROR)
{
printf("send() failed!\n");
}
else
{
end -= 1024;
}
shuchu1 = shuchu1 + 1024;
}*/


/*///10-14日完整接收数据
while (1)
{
nr = recv(sClient, (char*)point, 1056, 0);
if(nr > 0)
{
break;
}
}
x = point[0];
y = point[1];
ifn = 1;
while (1)
{
nr1 = nr1 + nr;
nr2 = nr1 / 8;
//nr = recv(sClient, (char*)(point+132*ifn), 1056, 0);
nr = recv(sClient, (char*)(point + nr2), 1056, 0);
if (nr <= 0) break;
ifn = ifn + 1;
}*/

