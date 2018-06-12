#!/usr/bin/env python
# encoding: utf-8

APPID      = 10                    #业务ID
SERVERHOST = '192.168.10.132'      #服务端ip
SERVERPORT = 9090                  #服务端端口
TIMEOUT    = 2000                  #ms(毫秒) 连接超时
MESSAGE    = 0                     #配置同一个key消息,在同一个时间有多个值如何操作   0:忽略直接插入  1：后一个值不插入  2：替换插入
FILENAME   = 'thriftclient.log'    #缓存文件名
LOGlEVEL   = 'info'                #error,只记录错误信息(默认)  info,记录错误信息和发送消息   debug,记录错误信息、发送信息和调试步骤
SETUP      = 300                   #消息时间间隔/秒（消息会根据这个配置，转换时间格式，如time=2016-01-27 16:19:11 会被转换为5分钟间隔的 2016-01-27 16:15:00）