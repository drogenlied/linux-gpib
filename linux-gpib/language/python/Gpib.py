
import gpib

RQS = (1<<11)
SRQ = (1<<12)
TIMO = (1<<14)


class Gpib:
	def __init__(self,name='gpib0'):
		self.id = gpib.find(name)


	def write(self,str):
		gpib.write(self.id, str)

	def read(self,len=512):
		self.res = gpib.read(self.id,len)
		return self.res

	def clear(self):
		gpib.clear(self.id)
		
	def wait(self,mask):
		gpib.wait(self.id,mask)
	
	def rsp(self):
		self.spb = gpib.rsp(self.id)
		return self.spb

	def trigger(self):
		gpib.trg(self.id)


	def ren(self,val):
		gpib.ren(self.id,val)
