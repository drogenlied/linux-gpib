#include <ibsys.h>

void osChngBase(gpib_device_t *device, unsigned long new_base)
{
	device->ibbase = new_base;
}

void osChngIRQ(gpib_device_t *device, int new_irq)
{
	device->ibirq = new_irq;
}

void osChngDMA(gpib_device_t *device, int new_dma)
{
	device->ibdma = new_dma;
}


