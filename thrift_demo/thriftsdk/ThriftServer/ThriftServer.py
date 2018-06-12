#!/usr/bin/env python
# encoding: utf-8
import sys ,redis ,json ,time, datetime, traceback ,logging

from ThriftInterface import Monitor
from ThriftInterface.ttypes import *

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol  import TBinaryProtocol
from thrift.server    import TServer

import config

logging.basicConfig(filename = 'thriftServer.log', 
                    filemode = 'a+', 
                    level    = logging.DEBUG,
                    format   = '%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s', 
                    datefmt  = '%Y-%m-%d %H:%M:%S')



class MonitorHandler:
    #默认redis地址和端口
    Redishost     = config.Redishost
    Redisport     = config.Redisport
    Redispassword = config.Redispassword

    def __init__(self):
        #连接redis数据库
        self.rediscon = redis.Redis(host=self.Redishost, port=self.Redisport)

    def messageFilter(self,Mess):
        '''客户端传递参数过滤'''
        try:
            if Mess.appid and Mess.subject and Mess.tag:
                message       = {}
                message['p']  = Mess.path
                message['ip'] = Mess.ip
                message['n']  = Mess.num
                message['i']  = Mess.info
                t  = time.strptime(Mess.time, "%Y-%m-%d %H:%M:%S")
                message['t']  = time.strftime("%Y-%m-%d %H:%M:%S",t)
                message['d']  = self.delayTime(Mess.sendtime)
                return message
        except Exception, e:
            logging.debug('messageFilter' + str(traceback.format_exc()))
            return 0

    def sendMessage(self, Mess, num):
        '''接收客户端消息，写入redis'''
        try:
            message       = self.messageFilter(Mess)
            #字典转json 写入redis
            if message:
                if Mess.model and Mess.unit and Mess.subject and Mess.tag:
                    key = '%s:%s:%s:%s' % (Mess.model, Mess.unit, Mess.subject, Mess.tag)
                elif Mess.unit and Mess.subject and Mess.tag:
                    key = '%s:%s:%s' % (Mess.unit, Mess.subject, Mess.tag)
                elif Mess.subject and Mess.tag:
                    key = '%s:%s' % (Mess.subject, Mess.tag)
                else:
                    logging.debug('key error' + str(Mess))
                    return 1
                decodejson = json.dumps(message, separators=(',',':'), sort_keys=True)
                if   num == 0:
                    self.insertRedis(key, message['t'], decodejson, int(Mess.appid))
                elif num == 1:
                    self.passRedis(key, message['t'], decodejson, int(Mess.appid))
                elif num == 2:
                    self.replaceRedis(key, message['t'], decodejson, int(Mess.appid))
        except Exception, e:
            logging.debug('sendMessage' + str(traceback.format_exc()))
            print traceback.format_exc()

    def insertRedis(self, key, sendtime, message, redisdb):
        '''key有相同时间，忽略直接插入'''
        try:
            self.rediscon.execute_command('select',redisdb)
            self.rediscon.rpush(key, message)
        except Exception, e:
            logging.debug('insertRedis' + str(traceback.format_exc()))

    def passRedis(self, key, sendtime, redisdb):
        '''key有相同时间，不插入'''
        try:
            self.rediscon.execute_command('select',redisdb)
            #判断新数据，是否大于最新的数据的时间，如果不是不插入
            v = self.rediscon.lrange(key, 0, 0)
            if v:
                lastmessage = json.loads(v[0])
                if lastmessage['t'] < sendtime:
                    self.rediscon.rpush(key, message)
                elif lastmessage['t'] == sendtime:
                    pass
            else:
                self.rediscon.rpush(key, message)
        except Exception, e:
            logging.debug('insertRedis' + str(traceback.format_exc()))

    def replaceRedis(self, key, sendtime, message, redisdb):
        '''key有相同时间，替换插入'''
        try:
            self.rediscon.execute_command('select',redisdb)
            #判断新数据，是否大于最新的数据的时间，如果不是不插入
            v = self.rediscon.lrange(key, 0, 0)
            if v:
                lastmessage = json.loads(v[0])
                if lastmessage['t'] < sendtime:
                    self.rediscon.rpush(key, message)
                elif lastmessage['t'] == sendtime:
                    self.rediscon.rpop(key)
                    self.rediscon.rpush(key, message)
            else:
                self.rediscon.rpush(key, message)
        except Exception, e:
            logging.debug('insertRedis' + str(traceback.format_exc()))

    def delayTime(self,sendtime):
        '''计算延迟时间'''
        try:
            t = time.strptime(sendtime, "%Y-%m-%d %H:%M:%S")
            date1 = datetime.datetime(*t[:7])
            date2 = datetime.datetime.now()
            d = date2-date1
            return d.seconds
        except Exception, e:
            return -1

class Servicer(object):
    """Servicer class"""
    serverport = config.SERVERPORT
    def __init__(self, Handlers):
        self.processor = Monitor.Processor(Handlers)
        self.transport = TSocket.TServerSocket(port = self.serverport)
        self.tfactory  = TTransport.TBufferedTransportFactory()
        self.pfactory  = TBinaryProtocol.TBinaryProtocolFactory()
        
    def run(self):
        #self.server = TServer.TThreadPoolServer(self.processor, self.transport, self.tfactory, self.pfactory)
        #self.server.setNumThreads(config.SERVERNUM)
        #多进程    #解决数据连接与插入不是一个在同一个数据库操作
        self.server = TServer.TForkingServer(self.processor, self.transport, self.tfactory, self.pfactory)
        print 'server start'
        self.server.serve()
        print 'server done'

if __name__ == '__main__':
    Handler = MonitorHandler()
    server = Servicer(Handler)
    server.run()

