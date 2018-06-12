/**
 * The first thing to know about are types. The available types in Thrift are:
 *  bool        Boolean, one byte
 *  byte        Signed byte
 *  i16         Signed 16-bit integer
 *  i32         Signed 32-bit integer
 *  i64         Signed 64-bit integer
 *  double      64-bit floating point value
 *  string      String
 *  binary      Blob (byte array)
 *  map<t1,t2>  Map from one type to another
 *  list<t1>    Ordered list of one type
 *  set<t1>     Set of unique elements of one type
 */
namespace cpp  monitor
namespace java monitor
namespace php  monitor
namespace py   monitor

//消息体
struct Message {
    1:  i32     appid,
    2:  string  model,
    3:  string  unit,
    4:  string  subject,
    5:  string  tag,
    6:  double  num,
    7:  string  info,
    8:  string  ip,
    9:  string  path,
    10: string  time,
    11: string  sendtime,
}
exception InvalidOperation {
    1: i32 whatOp,
    2: string why 
 }

//定义服务
service Monitor {
    //客户端发送单条消息,不监听返回信息 num=0是添加数据 num=1是替换 num=3是删除
    oneway void sendMessage(
        1: Message  message,
        2: i32 num
    ),
    i32 resendMessage (
        1: Message  message,
        2: i32 num
    ) throws (
        1:InvalidOperation ouch
    ),
    //客户端发送多条消息,不监听返回信息
    oneway void sendMessageList(
        1: list<Message> messages
    )
}