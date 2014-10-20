//#define NDEBUG // Debug supression

#include <os>
#include <iostream>

using namespace std;

uint8_t* buf = 0;
int bufsize = 0;
uint8_t* prev_data = 0;


void Service::start()
{
  cout << "*** Service is up - with OS Included! ***" << endl;    
 
  cout << "...Starting UDP server" << endl;

  //IP_stack& net = Dev::eth(0).ip_stack();
  auto& net = Dev::eth(0).ip_stack();
  /** @note: "auto net" would cause copy-construction (!) 
      since auto drops reference, const and volatile qualifiers. */
    
  //A one-way UDP server (a primitive test)
  net.udp_listen(8080,[&net](uint8_t* const data,int len){

      UDP::header* hdr = (UDP::header*)data;      
      

      int data_len = __builtin_bswap16(hdr->length);
      auto data_loc = data + sizeof(UDP::header);
      auto data_end = data + sizeof(UDP::header) + data_len;
      *data_end = 0; 
      // stringify (might mess up the ethernet trailer; oh well)
     
      debug("<APP SERVER> Got %i b of data (%i b frame) from %s:%i -> %s:%i\n",
            data_len, len, hdr->ip_hdr.ip.saddr.str().c_str(), 
            __builtin_bswap16(hdr->sport),
            hdr->ip_hdr.ip.daddr.str().c_str(), 
            __builtin_bswap16(hdr->dport));

      printf("%s", data_loc);                  
      
      // Craft response
      string response("Hello to you too!\n\n");
      bufsize = response.size() + sizeof(UDP::header);
      
      // Ethernet padding if necessary 
      if (bufsize < Ethernet::minimum_payload)
        bufsize = Ethernet::minimum_payload;
      
      buf = new uint8_t[bufsize];      
      strcpy((char*)buf + sizeof(UDP::header),response.c_str());
      
      
      // Respond
      debug("<APP SERVER> Sending %li b wrapped in %i b buffer \n",
            response.size(),bufsize);
      
      net.udp_send(hdr->ip_hdr.ip.daddr, hdr->dport, 
                   hdr->ip_hdr.ip.saddr, hdr->sport, buf, bufsize);
      
      // Free the buffer the next time around
      if (prev_data){
        free(prev_data);
      }
      prev_data = data;
          
      return 0;
    });
  
  cout << "<APP SERVER> Listening to UDP port 8080 " << endl;
  
  // Hook up to I/O events and do something useful ...
  
  cout << "Service out! " << endl; 
}
