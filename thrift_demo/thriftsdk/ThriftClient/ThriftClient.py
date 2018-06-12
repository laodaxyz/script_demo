#!/usr/bin/env python
# encoding: utf-8
import os, sys, time, socket, logging, traceback
import config

from ThriftInterface import Monitor
from ThriftInterface.ttypes import *

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol  import TBinaryProtocol

logfile = os.path.dirname(config.__file__)
logging.basicConfig(filename = logfile + '/' + str(config.FILENAME), 
                    filemode = 'a+', 
                    level    = logging.DEBUG,
                    format   = '%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s', 
                    datefmt  = '%Y-%m-%d %H:%M:%S')


class Client(object):
    """Client SDK"""
    appid     = config.APPID
    host      = config.SERVERHOST
    port      = config.SERVERPORT
    timeout   = config.TIMEOUT
    message   = config.MESSAGE
    filename  = str(logfile) + '/message.log'
    setup     = config.SETUP
    loglevel  = config.LOGlEVEL
    isconnet  = 0

    def __init__(self):
        if self.loglevel == 'debug':
            logging.debug('Client start')
        self.isconnet = self.serverConnet()
        if self.isconnet==0:
            logging.error("can't connet server %s:%d", self.host, self.port )
        else:
            if self.loglevel == 'debug':
                logging.debug('server connet success')
            #发送缓存信息
            self.sendoldmessage()

    def serverConnet(self):
        "连接thrift server"
        try:
            transport      = TSocket.TSocket(self.host, self.port)
            transport.setTimeout(self.timeout)
            self.transport = TTransport.TBufferedTransport(transport)
            self.protocol  = TBinaryProtocol.TBinaryProtocol(self.transport)
            self.client    = Monitor.Client(self.protocol)
            self.transport.open()
            return 1
        except Exception, e:
            return 0

    def serverClose(self):
        self.transport.close()
        if self.loglevel == 'debug':
            logging.debug('server end')
            logging.debug('Client end')

    def sendMessage(self, data):
        '''向服务端发送消息'''
        if self.isconnet == 0:
            self.setCache(data)
            return 101
        if self.loglevel == 'debug':
            logging.debug('sendMessage start')
        try:
            Mess = self.dataHandle(data)
            if Mess == 0:
                self.setCache(data)
                return 105
            self.client.sendMessage(Mess, self.message)
            if self.loglevel == 'debug':
                logging.debug('sendMessage end')
            return 200
        except Exception, e:
            #发送失败，本地缓存
            logging.error('sendMessage error: ' + str(traceback.format_exc()))
            self.setCache(data)
            return 201

    def dataHandle(self, data):
        '''消息字典转message对象'''
        try:
            if self.loglevel == 'debug' or self.loglevel == 'info':
                logging.info('message: ' + str(data))

            if not data.has_key('path'):
                data['path'] = str(os.path.abspath(sys.argv[0]))                     #文件路径
            if not data.has_key('ip'):
                data['ip']   = self.get_ip_address()[-1]            #获取eth0 ip
            if not data.has_key('appid'):
                data['appid'] = self.appid
            if not data.has_key('sendtime'):
                data['sendtime'] = time.strftime('%Y-%m-%d %H:%M:%S')   #发送时间

            Mess = Message()
            for (k,v) in data.iteritems():
                if k=='appid':
                    Mess.appid    = int(v)
                elif k=='model':
                    Mess.model    = str(v)
                elif k=='unit':
                    Mess.unit     = str(v)
                elif k=='subject':
                    Mess.subject  = str(v)
                elif k=='tag':
                    Mess.tag      = str(v)
                elif k=='num':
                    Mess.num      = float(v)
                elif k=='info':
                    Mess.info     = str(v)
                elif k=='time':
                    #按照时间频率取整，格式化日期
                    timeArray = time.strptime(v, "%Y-%m-%d %H:%M:%S")
                    timestamp= int(time.mktime(timeArray))
                    timestamp = (timestamp/self.setup)*self.setup
                    timeArray = time.localtime(timestamp)
                    v = time.strftime("%Y-%m-%d %H:%M:%S", timeArray)
                    Mess.time     = str(v)
                elif k=='sendtime':
                    Mess.sendtime = str(v)
                elif k=='path':
                    Mess.path     = str(v)
                elif k=='ip':
                    Mess.ip       = str(v)
            if Mess.appid and Mess.subject and Mess.tag:
                return Mess
        except Exception, e:
            logging.error('dataHandle: ' + str(traceback.format_exc()))
            logging.error('message error: ' + str(data))
            return 0
        
    def sendoldmessage(self):
        '''缓存发送至服务端'''
        if os.path.isfile(self.filename):
            if self.loglevel == 'debug':
                logging.debug('oldmessage start')
            fp = open(self.filename,'r')      #暂时不考虑多个进程同时读取文件
            AllLines = fp.readlines()
            for line in AllLines:
                self.sendMessage(eval(line))
            fp.close()
            os.remove(self.filename)         #读取内容后删除缓存文件
            if self.loglevel == 'debug':
                logging.debug('oldmessage end')

    def get_ip_address(self):
        '''获取本地ip列表'''
        localips = []
        if "linux" in sys.platform:
            import struct,fcntl
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            try:
                localips.append(socket.inet_ntoa(fcntl.ioctl(s.fileno(),0x8915,struct.pack('24s', 'eth0'))[20:24]))
            except:
                pass
        elif "cyg" in sys.platform:
            if sys.platform()[10:13] == '5.2':
                fp = os.popen("ipconfig | grep IP | awk -F':' '{print $2}'")
            elif sys.platform()[10:12] == '6.':
                fp = os.popen("ipconfig | grep IPv4 | awk -F':' '{print $2}'")
            localips = [ i[1:].strip() for i in fp.readlines()]
            fp.close()
        elif "win32" in sys.platform:
            localips = socket.gethostbyname_ex(socket.gethostname())[2]
        return localips

    def setCache(self, data):
        '''发送失败，本地缓存'''
        if isinstance(data,list):  #判断传递的是单条还是多条信息
            for x in data:
                fp = open(self.filename,'a+')
                m  = '%s\n' % str(x)
                fp.writelines(m)
        else:
            fp = open(self.filename,'a+')
            m  = '%s\n' % str(data)
            fp.writelines(m)
