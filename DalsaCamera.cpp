
#include "DalsaCamera.h"
#include <string.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

string DalsaCamera::DEVICE_NAME = "DLASA Camera";
string DalsaCamera::OBJECT_NAME = "DalsaCamera";

DalsaCamera::DalsaCamera(QObject *parent):QObject(parent)
{
	Acq = NULL;
   	Buffers = NULL;
   	Xfer = NULL;
   	XferPair = NULL;

   	RoiLeft = 0;
    RoiTop = 0;
    RoiWidth = DALSACEMERA::FULLIMAGE_WIDTH;
    RoiHeight = DALSACEMERA::FULLIMAGE_HEIGHT;
    mDalsaImage.data = NULL;
    globalDalsaImage.data = NULL;

    deviceState = DISCONNECTED;
    serialState = SERIAL_CLOSED;
    
    portname = string("/dev/corser/x64ExpCL4x1_s0");
    dalsaSerial = NULL;
}

DalsaCamera::~DalsaCamera()
{
	if (dalsaSerial != NULL){
		delete dalsaSerial;
		dalsaSerial = NULL;
	}
    if (mDalsaImage.data != NULL){
        delete mDalsaImage.data;
        mDalsaImage.data = NULL;
    }
}

bool DalsaCamera::Connect()
{
	try{
    	dalsaSerial = new QSerial("/dev/corser/x64ExpCL4x1_s0");
		dalsaSerial->Open_Port();

		if (Test_Connected()){
			serialState = SERIAL_CONNECTED;
		}
		else{
			return false;
		}
	}catch(QException e){
        cout<<e.getMessage()<<endl;
        return false;
		//throw e;//向上抛出异常，等待上层处理
	}

	//设置缓冲区，开始接受数据
	char acqServerName[128], camFilename[128], vicFilename[128];
	strcpy(acqServerName, "Xcelera-CL_PX4_1");
	strcpy(camFilename, "D_FA-80-4M180_8-bits_Default.ccf");
	strcpy(vicFilename, "D_FA-80-4M180_8-bits_Default.ccf");
	UINT32 acqDeviceNumber = 0;
	
	SapLocation loc(acqServerName, acqDeviceNumber);
   	Acq = new SapAcquisition(loc, camFilename, vicFilename);//分配SapAcqusition对象，用于图像获取
   	Buffers = new SapBuffer(MAX_IMAGE_BUFFERS, Acq);//分配SapBuffer对象，用于图像缓存
   	XferPair = new SapXferPair(Acq, Buffers);
   	if(XferPair->SetFlipMode(SapXferPair::FlipVertical)){
		cout<<"set flip mode is vertical"<<endl;
   	}
   	
   	//将拍摄的完整的图像保存在Buffer中，一旦一幅图像被transfer都会调用回调函数AcqCallback函数
	//Xfer = new SapAcqToBuf(Acq, Buffers, AcqCallback, &context);//这条程序意义重大！
	Xfer = new SapTransfer(AcqCallback, &context, loc);
	Xfer->AddPair(*XferPair);
	
   	if (Acq!=NULL && !(Acq->Create())){//创建SapAcqusition对象
       	cout <<"fail to create acquisition"<<endl;
		Disconnect();
		return false;
	}
   	if (Buffers!=NULL && !(Buffers->Create())){//创建SapBuffer对象
   		cout <<"fail to create buffer"<<endl;
		Disconnect();
		return false;
	}
   	if (Xfer!=NULL && !(Xfer->Create())){//创建SapTransfer对象
   		cout <<"fail to create transfer"<<endl;
		Disconnect();
		return false;
	}

	//get parameters
    Acq->GetParameter(CORACQ_PRM_CROP_HEIGHT, &context.height);
    Acq->GetParameter(CORACQ_PRM_CROP_WIDTH, &context.width);
    Acq->GetParameter(CORACQ_PRM_OUTPUT_FORMAT, &context.pixelFormat);
    Acq->GetParameter(CORACQ_PRM_PIXEL_DEPTH, &context.pixelDepth);
    for (int i=0; i<MAX_IMAGE_BUFFERS; i++){
		Buffers->GetParameter(i, CORBUFFER_PRM_ADDRESS, &context.image[i]);
	}
	context.Buffers = Buffers;
	context.camera = this;

	if (Xfer->Grab()){
		deviceState = CONNECTED;
		cout << "Connected to Dalsa Camera successfully"<<endl;
		return true;
	}
	else{
		Disconnect();
		return false;
	}
}

bool DalsaCamera::IsConnected()
{
	return (/*serialState == SERIAL_CONNECTED &&*/ deviceState == CONNECTED);
}

void DalsaCamera::Disconnect()
{
    if (XferPair != NULL){
        delete XferPair;
        XferPair = NULL;
    }
	if (Xfer != NULL){
    	Xfer->Freeze();
		Xfer->Destroy();	
		delete Xfer;
		Xfer = NULL;
	}
	if (Buffers != NULL){
		Buffers->Destroy();
		delete Buffers;
		Buffers = NULL;
	}
	if (Acq != NULL){
		Acq->Destroy();
		delete Acq;
		Acq = NULL;
	}
	if (dalsaSerial!=NULL && dalsaSerial->Get_SerialStatus() != SERIAL_CLOSED){
		dalsaSerial->Close_Port();
	}
	deviceState = DISCONNECTED;
}

//Initial connection
 bool DalsaCamera::Test_Connected()
 {
     unsigned int times = 0;
     bool success = false;
     char data[MAX_BUFFER_SIZE];

     if (dalsaSerial == NULL){
     	return false;
     }
     while (times < MAX_TEST_CONNECTION_TIMES){
     	dalsaSerial->Write_Port("\x1B\n\r"); //Send ESC character
     	usleep(DALSE_SERIAL_QUERY_DELAY);    
		dalsaSerial->Read_Port(data);        //Receive the returned data

		dalsaSerial->Write_Port("\n\r");     //Send the null character
		usleep(DALSE_SERIAL_QUERY_DELAY);

        // start: extra settings for NIR(850nm) LED source
		dalsaSerial->Write_Port("sli L3 ON\n\r");
		usleep(DALSE_SERIAL_QUERY_DELAY);
		
		dalsaSerial->Write_Port("sos L3 1\n\r");
		usleep(DALSE_SERIAL_QUERY_DELAY);
		
    	dalsaSerial->Write_Port("soy L3 1\n\r");
		usleep(DALSE_SERIAL_QUERY_DELAY);
		// end: extra settings for IR LED source
		
		//Analyse the returned data
		int numBytes = dalsaSerial->Read_Port(data);
		if (ValidateValue(data, numBytes)){
			success = true;
			break;
		}
     	++times;
     }
     dalsaSerial->flush();
     usleep(2*DALSE_SERIAL_QUERY_DELAY);
     dalsaSerial->Read_Port(data);

     return success;
 }
 
void DalsaCamera::AcqCallback(SapXferCallbackInfo *pInfo)
{
    PMY_CONTEXT pContext = (PMY_CONTEXT) pInfo->GetContext();
    DalsaCamera *dalsaCamera = pContext->camera;
    if (!pInfo->IsTrash()){
        dalsaCamera->Excute();
   	}
}

void DalsaCamera::ExtractROIFromImage(uchar* src, UINT32 width, UINT32 height, UINT32 x_offset, UINT32 y_offset, UINT32 rowBytes)
{
    if (mDalsaImage.data != NULL){
        mDalsaImage.cols = width;
        mDalsaImage.rows = height;
        uchar *dst = mDalsaImage.data, *src_start = src + y_offset*rowBytes;

        //Copy data
        int LoopHeight = height>>4;
        int NewTotalHeight = LoopHeight<<4;

        for(int i=0; i<LoopHeight; ++i){
            int base = i<<4;
            memcpy(dst + base*width,     src_start + base*rowBytes + x_offset, width);     //1st line
            memcpy(dst + (base+1)*width, src_start + (base+1)*rowBytes + x_offset, width); //2nd line
            memcpy(dst + (base+2)*width, src_start + (base+2)*rowBytes + x_offset, width); //3rd line
            memcpy(dst + (base+3)*width, src_start + (base+3)*rowBytes + x_offset, width); //4th line
            memcpy(dst + (base+4)*width, src_start + (base+4)*rowBytes + x_offset, width); //5th line
            memcpy(dst + (base+5)*width, src_start + (base+5)*rowBytes + x_offset, width); //6th line
            memcpy(dst + (base+6)*width, src_start + (base+6)*rowBytes + x_offset, width); //7th line
            memcpy(dst + (base+7)*width, src_start + (base+7)*rowBytes + x_offset, width); //8th line
            memcpy(dst + (base+8)*width, src_start + (base+8)*rowBytes + x_offset, width); //9th line
            memcpy(dst + (base+9)*width, src_start + (base+9)*rowBytes + x_offset, width); //10th line
            memcpy(dst + (base+10)*width, src_start + (base+10)*rowBytes + x_offset, width); //11th line
            memcpy(dst + (base+11)*width, src_start + (base+11)*rowBytes + x_offset, width); //12th line
            memcpy(dst + (base+12)*width, src_start + (base+12)*rowBytes + x_offset, width); //13th line
            memcpy(dst + (base+13)*width, src_start + (base+13)*rowBytes + x_offset, width); //14th line
            memcpy(dst + (base+14)*width, src_start + (base+14)*rowBytes + x_offset, width); //15th line
            memcpy(dst + (base+15)*width, src_start + (base+15)*rowBytes + x_offset, width); //16th line
        }

        int base = NewTotalHeight;
        for(int i=0; i<height-NewTotalHeight; ++i){
            memcpy(dst + (base+i)*width, src_start + (base+i)*rowBytes + x_offset, width); //the remain lines
        }
    }
}

void DalsaCamera::Excute()
{
    // Check Roileft,RoiTop, RoiWidth, RoiHeight
    if ((RoiLeft+RoiWidth)>DALSACEMERA::FULLIMAGE_WIDTH || (RoiTop+RoiHeight)>DALSACEMERA::FULLIMAGE_HEIGHT){
        return;
    }
    if (mDalsaImage.data == NULL){
        mDalsaImage.data = new uchar[DALSACEMERA::FULLIMAGE_WIDTH*DALSACEMERA::FULLIMAGE_HEIGHT];
    }
    if (globalDalsaImage.data == NULL){
        globalDalsaImage.data = new uchar[DALSACEMERA::FULLIMAGE_WIDTH*DALSACEMERA::FULLIMAGE_HEIGHT];
    }

    int buf_index = context.Buffers->GetIndex();
    int counterstamp;
    context.Buffers->GetCounterStamp(&counterstamp);
    
    ++globalDalsaImage.count;
    globalDalsaImage.buffer_index = buf_index;
    globalDalsaImage.counterStamp = counterstamp;
    globalDalsaImage.rows = RoiHeight;
    globalDalsaImage.cols = RoiWidth;

    // Copy image ROI for display
    ExtractROIFromImage((uchar*)context.image[buf_index], RoiWidth, RoiHeight, RoiLeft, RoiTop, DALSACEMERA::FULLIMAGE_WIDTH);
    
    read_write_locker.lockForWrite();
    uchar* tmp = globalDalsaImage.data;
    globalDalsaImage.data = mDalsaImage.data; // Store the latest image data
    mDalsaImage.data = tmp;
    read_write_locker.unlock();

    emit Broadcast_Image_Recieved();//向上层广播图像已获得信号
}

bool DalsaCamera::GetExposureTime(UINT64 &value)
{
	char data[MAX_BUFFER_SIZE];
	
	if (serialState != SERIAL_CONNECTED){ return false; }
	dalsaSerial->Write_Port("get set\n\r");
	
	usleep(DALSE_SERIAL_QUERY_DELAY);
	int numBytes = dalsaSerial->Read_Port(data);
	
	//Analyse the returned data
	int success = ValidateValue(data, numBytes);
	//cout<<"Get ExposureTime: "<<success<<endl;
    if(success){
    	if ((CameraValue[0] >= '0' && CameraValue[0] <= '9') || (CameraValue[0] == '.')){
	    	double tempValue = atof(CameraValue);
	    	if (tempValue < 0){
	    		value = 0;
	    		return false;
	    	}
	    	else {
	    		value = tempValue;
	    		return true;
	    	}
	    }
    }
    else{
        return false; 
    }
}

void DalsaCamera::SetExposureTime(UINT64 exposureTime)
{
	char command[MAX_BUFFER_SIZE];
	if (serialState != SERIAL_CONNECTED){ return; }

	sprintf(command, "set %ld\n\r", exposureTime);
	dalsaSerial->Write_Port(command);
	
	// setting: extra command for NIR(850nm) LED source
	/*usleep(DALSE_SERIAL_QUERY_DELAY);
	sprintf(command, "sod L3 %ld\n\r", exposureTime);
	dalsaSerial->Write_Port(command);*/
}

bool DalsaCamera::GetFrameRate(UINT32 &value)
{
	char data[MAX_BUFFER_SIZE];
	
	if (serialState != SERIAL_CONNECTED){ return false; }
	dalsaSerial->Write_Port("get ssf\n\r");
	
	usleep(DALSE_SERIAL_QUERY_DELAY);
	int numBytes = dalsaSerial->Read_Port(data);

	//Analyse the returned data
	int success = ValidateValue(data, numBytes);
	//cout<<"Get frameRate: "<<success<<endl;
    if(success){
    	if ((CameraValue[0] >= '0' && CameraValue[0] <= '9') || (CameraValue[0] == '.')){
	    	double tempValue = atof(CameraValue);
	    	if (tempValue < 0){
	    		value = 0;
	    		return false;
	    	}
	    	else {
	    		value = tempValue;
	    		return true;
	    	}
	    }
    }
    else{
        return false; 
    }
}

void DalsaCamera::SetFrameRate(UINT32 frameRate)
{
	char command[MAX_BUFFER_SIZE];
	if (serialState != SERIAL_CONNECTED){ return; }

	sprintf(command, "ssf %d\n\r", frameRate);
	dalsaSerial->Write_Port(command);
}

void DalsaCamera::SetTriggerMode(DALSA_TRIGGERMODE mode)
{
    if (serialState != SERIAL_CONNECTED){ return; }
    
    if (mode == INTERNAL){
        dalsaSerial->Write_Port("stm i\n\r");
        usleep(DALSE_SERIAL_QUERY_DELAY);
        dalsaSerial->Read_Port(CameraValue);

        dalsaSerial->Write_Port("sts CC1\n\r");//set trigger source to CC1
        usleep(DALSE_SERIAL_QUERY_DELAY);
        dalsaSerial->Read_Port(CameraValue);
    }
    else if (mode == EXTERNAL){
        dalsaSerial->Write_Port("stm e\n\r");
        usleep(DALSE_SERIAL_QUERY_DELAY);
        dalsaSerial->Read_Port(CameraValue);

        dalsaSerial->Write_Port("sem w\n\r");
        usleep(DALSE_SERIAL_QUERY_DELAY);
        dalsaSerial->Read_Port(CameraValue); 

        dalsaSerial->Write_Port("sts L1\n\r"); //set trigger source to L1
        usleep(DALSE_SERIAL_QUERY_DELAY);
        dalsaSerial->Read_Port(CameraValue);
    }
}

bool DalsaCamera::GetTriggerMode(DALSA_TRIGGERMODE &mode)
{
     if (serialState != SERIAL_CONNECTED){ return false; }
     char data[MAX_BUFFER_SIZE];
     
     //Send query command
     dalsaSerial->Write_Port("get stm\n\r");
     usleep(DALSE_SERIAL_QUERY_DELAY);
     
     //Receive the returned data
     int numBytes = dalsaSerial->Read_Port(data);

     if (ValidateValue(data, numBytes)){
     	if (strlen(CameraValue) == 1){
     		if (CameraValue[0] == 'i'){
     			mode = INTERNAL;
     			return true;
     		}
     		else if (CameraValue[0] == 'e'){
     			mode = EXTERNAL;
     			return true;
     		}
     	}
        return false;
     }
     else
         return false;
} 

//验证返回值是否有效
bool DalsaCamera::ValidateValue(char *data, int num)
{
	bool success = false;
	int index = 0;

	cout<<"Receive bytes num: "<<num<<endl;
	for (int i=0; i<num; ++i){
		if (data[i] == '\n')
			cout<<"\\n";
		else if(data[i] == '\r')
			cout<<"\\r";
		else
			cout<<data[i];
	}
	cout<<"\n";
	
    memset(CameraValue, 0, sizeof(CameraValue));
    for (int i=0; i<num-2; ++i){
    	if (strncmp(data+i,"OK>",3) == 0){
    		success = true;
    		break;
    	}
    	else if ((data[i] >= '0' && data[i] <= '9') || (data[i] == '.') || (data[i] >= 'a' && data[i] <= 'z'))
    		CameraValue[index++] = data[i];
    }

    return success;
}
