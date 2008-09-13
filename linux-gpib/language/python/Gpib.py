
import gpib

RQS = (1<<11)
SRQ = (1<<12)
TIMO = (1<<14)


class Gpib:
	'''Three ways to create a Gpib object:
	Gpib("name")
		returns a board or device object, from a name in the config file
	Gpib(board_index)
		returns a board object, with the given board number
	Gpib(board_index, pad[, sad[, timeout[, send_eoi[, eos_flags[, eos_char]]]]])
		returns a device object, like ibdev()'''
	
	def __init__(self, name = 'gpib0', pad = None, sad = 0, timeout = 13, send_eoi = 1, eos_flags = 0, eos_char = '\x0a'):
		self._own = False
		if isinstance(name, basestring):
			self.id = gpib.find(name)
			self._own = True
		elif pad is None:
			self.id = name
		else:
			self.id = gpib.dev(name, pad, sad, timeout, send_eoi, eos_flags, eos_char)
			self._own = True
	
	# automatically close descriptor when instance is deleted
	def __del__(self):
		if self._own:
			gpib.close(self.id)
			
	def __repr__(self):
		return "%s(%d)" % (self.__class__.__name__, self.id)


	def cmd(self,str):
		gpib.cmd(self.id, str)
	
	def config(self,option,value):
		self.res = gpib.config(self.id,option,value)
		return self.res
	
	def ifc(self):
		gpib.ifc(self.id)
	
	def write(self,str):
		gpib.write(self.id, str)

	def write_async(self,str):
		gpib.write_async(self.id, str)
	
	def read(self,len=512):
		self.res = gpib.read(self.id,len)
		return self.res

	def listener(self,pad,sad=0):
		self.res = gpib.listener(self.id,pad,sad)
		return self.res

	def ask(self,option):
		self.res = gpib.ask(self.id,option)
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

	def ibsta(self):
		self.res = gpib.ibsta()
		return self.res

	def ibcnt(self):
		self.res = gpib.ibcnt()
		return self.res

	def tmo(self,value):
		return gpib.tmo(self.id,value)
