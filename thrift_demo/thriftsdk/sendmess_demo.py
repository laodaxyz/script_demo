#!/usr/bin/env python
# encoding: utf-8

'''
这个文件是demo实例   发送消息时需如下基本
1，引入 ThriftClient模块下的 Client 并实例化
2，将消息写入一个字典
3，调用sendMessage发送消息

错误的信息会被记录到当前文件的thriftClient.log
发送失败的消息会被缓存在message.log，在下一次 ThriftClient.Client实例化时发送
'''

from ThriftClient.ThriftClient import Client

if __name__ == '__main__':
    message            = {}
    message['model']   = 'model'
    message['unit']    = 'unit'
    message['subject'] = 'subject'
    message['tag']     = 'tag'
    message['num']     = 1110
    message['info']    = 'info'
    message['time']    = '2016-05-05 16:54:00'

    ClientClass = Client()
    print ClientClass.sendMessage(message)
    ClientClass.serverClose()