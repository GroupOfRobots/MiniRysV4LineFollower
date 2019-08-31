#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <ctime>
#include <chrono>
using namespace cv;
using namespace std;
using boost::asio::ip::udp;

double scaleFactor = 2;
int DELAY_CAPTION = 1500;
int DELAY_BLUR = 100;
int MAX_KERNEL_LENGTH = 31;

Mat src; Mat dst;
int chunkSize = 64000;
/// Function headers
int display_dst( int delay, char* window_name);

int main(int argc, char* argv[])
{
  try
  {
      std::cerr << "ok1" << std::endl;
    if (argc != 2)
    {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    boost::asio::io_service io_service;

    udp::resolver resolver(io_service);
    udp::resolver::query query(udp::v4(), argv[1], "2024");
    udp::endpoint receiver_endpoint = *resolver.resolve(query);
    udp::socket socket(io_service);
    socket.open(udp::v4());
    boost::array<char, 1> send_buf  = {{ 0 }};
    boost::array<char, 64000> recv_buf;
    udp::endpoint sender_endpoint;
    std::size_t bytesReceived =1;

    auto start = chrono::steady_clock::now();
    
    int first = 1;
    while(bytesReceived != 0){ 
      socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);     
      bytesReceived= socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);
      
      const boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
			// Get the time offset in current day
    	const boost::posix_time::time_duration td = now.time_of_day();
			const long hours = td.hours();
    	const long minutes = td.minutes();
    	const long seconds = td.seconds();
    	const long milliseconds = td.total_milliseconds() - ((hours * 3600 + minutes * 60 + seconds) * 1000);
			char buf[40];
    	sprintf(buf, "%02ld:%02ld:%02ld.%03ld", hours, minutes, seconds, milliseconds);
    	cout<<"Receive time: "<<buf<<endl;

      if(first == 1){
        auto end = chrono::steady_clock::now();
        first = 0;
        cout << "Transfer time in microseconds: " << chrono::duration_cast<chrono::milliseconds>(end - start).count()/2
				<< " ms" << endl;
      }

      //std::cout.write(recv_buf.data(), bytesReceived);
      if(recv_buf.empty()){std::cerr << "Buf is empty, got " <<bytesReceived << "bytes"<< std::endl;}

      std:vector<char> data(recv_buf.begin(), recv_buf.end());
      dst=imdecode(data,1);

	    resize(dst, dst, Size(0,0), scaleFactor, scaleFactor, 1);
      display_dst(27,"Received");	
	}
	destroyAllWindows();
    

  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

 int display_dst( int delay, char* window_name )
  {
    imshow( window_name, dst );
    int c = waitKey ( delay );
    if( c >= 0 ) { return -1; }
    return 0;
  }
