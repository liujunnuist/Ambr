#include "connmgt.h"

void Ambr::P2P::ConnMgt::start()
{
  accept_conn();
  auto address = Ambr::P2P::NetAddress::SelectAddress(1);
  for (int i = 0; i < 3; i++) {
    NetBase netbase(io_client_);
    if (!netbase.ConnectSocket(*address[i])) {
      continue;
    }

    if (Ambr::P2P::Peer::ValidateAddr(netbase.socket())) {
      peers_.push_back(Ambr::P2P::Peer(std::move(netbase.socket())));
    }
    else {
      netbase.CloseSocet();
    }
  }
  }
}

void Ambr::P2P::ConnMgt::accept_conn()
{
  acceptor_.async_accept(
    [this](boost::system::error_code ec, ba::ip::tcp::socket socket)
  {
    if (!ec)
    {
      std::cout << socket.remote_endpoint().address().to_string() << std::endl;
      std::make_shared<Session>(std::move(socket))->Read();
    }
    else {
      socket.close();
    }

    accept_conn();
  });
}

void Ambr::P2P::ConnMgt::Session::Read()
{
  auto self(shared_from_this());
  socket_.async_read_some(boost::asio::buffer(buff_, Ambr::P2P::NetBase::BUFFSIZE),
    [this, self](boost::system::error_code ec, std::size_t len)
  {
    if (!ec)
    {
      std::stringstream stream;
      stream.write(buff_, len);
      auto msg = Ambr::P2P::Peer::ProcessMessage(stream.str());
    }
  });
}

void Ambr::P2P::ConnMgt::Session::Write(std::string data)
{
  auto self(shared_from_this());
  //TODO can not use like this, check 
  //boost::asio::buffer(data);

  boost::asio::async_write(socket_, boost::asio::buffer(data.c_str(), data.size()),
    [this, self](boost::system::error_code ec, std::size_t /*length*/)
  {
    if (!ec)
    {
    }
  });
}


using boost::asio::ip::tcp;
int main(int argc, char* argv[])
{
  boost::asio::io_context io;
  try {
    //Ambr::P2P::ConnMgt conn(io);
    boost::system::error_code ec;
    //std::thread t(boost::bind(&boost::asio::io_context::run, &io));
 
    
    Ambr::P2P::NetMsg netmsg;
    netmsg.SerializeMany("143243243", "1433214321");
    netmsg.Encode_header();
   

    char buff[1024];
    tcp::socket socket(io);
    tcp::resolver resolver(io);
    auto ep = resolver.resolve("localhost", "8000");
    boost::asio::connect(socket, ep, ec);
    if (!ec) {
      boost::asio::write(socket, boost::asio::buffer(netmsg.buffer_, netmsg.npos_));
      netmsg.Clear();
      netmsg.SerializeMany("143243243");
      netmsg.Encode_header();
      boost::asio::write(socket, boost::asio::buffer(netmsg.buffer_, netmsg.npos_));
      //auto len = boost::asio::read(socket, boost::asio::buffer(buff, 1024));
     // std::cout.write(buff, len);
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  
}

