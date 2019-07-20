
#ifndef _PERCIPIO_CAMPORT_HPP_
#define _PERCIPIO_CAMPORT_HPP_
#include <cstring>
#include <vector>
#include <cassert>
#include <TY_API.h>
#include "percipio_camport.h"


#define assert_ok(x) do{ \
    int err = (x); \
    if(err != TY_STATUS_OK){ \
        printf("Assert failed: %d (%s), At: %s line %d\n", err, TYErrorString(err), __FILE__, __LINE__);\
    }\
}while(0)


namespace percipio {


static const DeviceProperty prop_list[] = {
  { PROP_DEVICE_INFO,               "Device Info",                PROP_TYPE_STRUCT },
  { PROP_DEVICE_ADDR,               "Device Address",             PROP_TYPE_STRING },
  { PROP_CMOS_REG,                  "CMOS Register Info",         PROP_TYPE_STRUCT },
  { PROP_LASER_POW,                 "Laser Power",                PROP_TYPE_INT8 },
  { PROP_WORKMODE,                  "Device Workmode",            PROP_TYPE_INT8 },
  { PROP_DEPTH_RESOLUTION,          "Depth Resolution",           PROP_TYPE_ENUM },
  { PROP_CALIB_DEPTH_INTRINSIC,     "Depth Intrinsics",           PROP_TYPE_STRUCT },
  { PROP_POINT_CLOUD_OUTPUT,        "Point Cloud Output",         PROP_TYPE_BOOL },
  { PROP_FRAME_READY_CALLBACK,      "Frame Ready Callback",       PROP_TYPE_OTHER },
  { PROP_CALLBACK_USER_DATA,        "Callback User Data",         PROP_TYPE_OTHER },
  { PROP_SPECKLE_FILTER,            "Speckle Filter",             PROP_TYPE_STRUCT },
  { PROP_UNDISTORT_IR,              "IR Undistort",               PROP_TYPE_BOOL },
  { PROP_COORDINATE_MAP,            "Coordinate mapper",          PROP_TYPE_OTHER },
  { PROP_TRIGGER_MODE,              "Trigger Mode",               PROP_TYPE_BOOL },
  { PROP_AUTO_GAIN_CTRL,            "Camera Auto Gain Control",   PROP_TYPE_STRUCT },
};

static int32_t CAM_DATA_TO_TY_COMPONENT(int cam_data_type)
{
    switch(cam_data_type){
        case CAMDATA_LEFT:    return TY_COMPONENT_IR_CAM_LEFT;
        case CAMDATA_RIGHT:   return TY_COMPONENT_IR_CAM_RIGHT;
        case CAMDATA_COLOR:   return TY_COMPONENT_RGB_CAM;
        case CAMDATA_DEPTH:   return TY_COMPONENT_DEPTH_CAM;
        case CAMDATA_POINT3D: return TY_COMPONENT_POINT3D_CAM;
        default: return -1;
    }
}

static ImageBuffer::PixelTypes TY_PIXEL_TO_OLD_PIXEL(int32_t tyPixelType)
{
    switch(tyPixelType){
        case TY_PIXEL_FORMAT_MONO: return ImageBuffer::PIX_8C1;
        case TY_PIXEL_FORMAT_RGB: return ImageBuffer::PIX_8C3;
        case TY_PIXEL_FORMAT_YUYV: return ImageBuffer::PIX_8C2;
        case TY_PIXEL_FORMAT_YVYU: return ImageBuffer::PIX_8C2;
        case TY_PIXEL_FORMAT_DEPTH16: return ImageBuffer::PIX_16C1;
        case TY_PIXEL_FORMAT_FPOINT3D: return ImageBuffer::PIX_32FC3;
        default: return (ImageBuffer::PixelTypes)-1;
    }
}

////////////////////////////////////////////////////////////////////////////

class CoordinateMapper : public ICoordinateMapper
{
    TY_DEV_HANDLE m_hDevice;
    CamIntrinsic  m_oldIntrinsic;
public:
    CoordinateMapper(TY_DEV_HANDLE hDevice)
        : m_hDevice(hDevice)
    {
        TY_CAMERA_INTRINSIC intr;
        int32_t w, h;
        m_oldIntrinsic.width = 0;
        m_oldIntrinsic.height = 0;
        if (TYGetStruct(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_STRUCT_CAM_INTRINSIC, &intr, sizeof(intr)) != TY_STATUS_OK){
            return;
        }
        if (TYGetInt(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_INT_WIDTH, &w) != TY_STATUS_OK){
            return;
        }
        if (TYGetInt(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_INT_HEIGHT, &h) != TY_STATUS_OK){
            return;
        }
        m_oldIntrinsic.width = w;
        m_oldIntrinsic.height = h;
        memcpy(m_oldIntrinsic.data, intr.data, sizeof(intr));
    }
    ~CoordinateMapper()
    {
    }
    const CamIntrinsic* get_depth_intrinsics()
    {
        return &m_oldIntrinsic;
    }
    int DepthToWorld(const Vect3f *depth, Vect3f *world)
    {
        float fx = m_oldIntrinsic.data[0];
        float fy = m_oldIntrinsic.data[4];
        float cx = m_oldIntrinsic.data[2];
        float cy = m_oldIntrinsic.data[5];
        world->x = (depth->x - cx) * depth->z / fx;
        world->y = (depth->y - cy) * depth->z / fy;
        world->z = depth->z;

        return CAMSTATUS_SUCCESS;
    }
    int DepthToWorld(const ImageBuffer *depth, ImageBuffer *cloud)
    {
        Vect3f _depth;
        float* pZ = (float*)depth->data;
        Vect3f* pW = (Vect3f*)cloud->data;
        float fx = m_oldIntrinsic.data[0];
        float fy = m_oldIntrinsic.data[4];
        float cx = m_oldIntrinsic.data[2];
        float cy = m_oldIntrinsic.data[5];
        for(int row = 0; row < depth->height; row++){
            _depth.y = row;
            for(int col = 0; col < depth->width; col++){
                _depth.x = col;
                _depth.z = *pZ;
                pW->x = (_depth.x - cx) * _depth.z / fx;
                pW->y = (_depth.y - cy) * _depth.z / fy;
                pW->z = _depth.z;
                pZ++;
                pW++;
            }
        }
        return CAMSTATUS_SUCCESS;
    }

    int WorldToDepth(const Vect3f *world, Vect3f *depth)
    {
        float fx = m_oldIntrinsic.data[0];
        float fy = m_oldIntrinsic.data[4];
        float cx = m_oldIntrinsic.data[2];
        float cy = m_oldIntrinsic.data[5];
        depth->x = world->x * fx / world->z + cx;
        depth->y = world->y * fy / world->z + cy;
        depth->z = world->z;
        return CAMSTATUS_SUCCESS;
    }
};

class CameraVideoSourceWrapper : public ICameraVideoSource
{
    std::vector<TY_DEVICE_BASE_INFO>  m_devInfos;
    TY_DEV_HANDLE                     m_hDevice;
    TY_FRAME_DATA                     m_frame;
    TY_DEVICE_BASE_INFO               m_curDevInfo;
    CoordinateMapper*                 m_pCoordinateMapper;
    int32_t                           m_Point3DComponent;
    EventCallbackFunc                 m_callback;
    void*                             m_userdata;
    std::vector<uint8_t>              m_fb0;
    std::vector<uint8_t>              m_fb1;

public:
    CameraVideoSourceWrapper() :   
            m_hDevice(0)
          , m_pCoordinateMapper(NULL)
          , m_Point3DComponent(0)
          , m_callback(NULL)
          , m_userdata(NULL)
    {
        m_frame.userBuffer = NULL;
        //TYInitLib();
        updateCamList();
    }

    ~CameraVideoSourceWrapper(){  }

    int GetDeviceNum()
    {
        updateCamList();
        return m_devInfos.size();
    }

    int GetPropertyNum()
    {
        return sizeof(prop_list)/sizeof(DeviceProperty);
    }

    int GetDeviceList(int *devs)
    {
        int32_t n = m_devInfos.size();
        for(int i = 0; i < n; i++)
        {
            devs[i] = i+1;
        }
        return CAMSTATUS_SUCCESS;
    }

    int GetPropertyList(DeviceProperty *device_prop)
    {
        memcpy(device_prop, prop_list, sizeof(prop_list));
        return CAMSTATUS_SUCCESS;
    }

    //trigger device to get a frame async
    CameraSourceStatus TriggerOnce()
    {
        assert_ok(TYSendSoftTrigger(m_hDevice));
        return CAMSTATUS_SUCCESS;
    }

    TY_DEV_HANDLE get_handle(){
      return  m_hDevice;
    }

    CameraSourceStatus FramePackageGet()
    {
        if(m_callback){
            // if this is called from callback, return OK immediately
            return CAMSTATUS_SUCCESS;
        }
    
        if(m_frame.userBuffer){
            assert_ok(TYEnqueueBuffer(m_hDevice, m_frame.userBuffer, m_frame.bufferSize));
            m_frame.userBuffer = NULL;
        }
        assert_ok(TYFetchFrame(m_hDevice, &m_frame, -1));
        return CAMSTATUS_SUCCESS;
    }
    
    CameraSourceStatus FrameGet(int cam_data_type, ImageBuffer *buff)
    {
        int component = CAM_DATA_TO_TY_COMPONENT(cam_data_type);
        if(component < 0){
            return CAMSTATUS_PARAM_INVALID;
        }
        for(int i = 0; i < m_frame.validCount; i++){
            if(component == m_frame.image[i].componentID){
                buff->width = m_frame.image[i].width;
                buff->height = m_frame.image[i].height;
                buff->type = TY_PIXEL_TO_OLD_PIXEL(m_frame.image[i].pixelFormat);
                buff->timestamp = m_frame.image[i].timestamp;
                buff->frame_index = 0;
                buff->data = (unsigned char*)m_frame.image[i].buffer;
                //YUYV or YVYU ? need one pixel memory space for store original format for old app.
                //highly recommmand to use new API
                buff->data[0] = m_frame.image[i].pixelFormat;
                return CAMSTATUS_SUCCESS;
            }
        }
        return CAMSTATUS_NODATA;
    }
    
    CameraSourceStatus OpenDevice(int id)
    {
        if(id <=0 || id > m_devInfos.size()){
            return CAMSTATUS_PARAM_INVALID;
        }
        if(TYOpenDevice(m_devInfos[id-1].id, &m_hDevice) != TY_STATUS_OK){
            return CAMSTATUS_ERROR;
        }
        m_curDevInfo = m_devInfos[id - 1];
        assert(m_pCoordinateMapper == NULL);
        m_pCoordinateMapper = new CoordinateMapper(m_hDevice);
        return CAMSTATUS_SUCCESS;
    }
    
    CameraSourceStatus OpenDevice(const char *ip_address)
    {
      if (ip_address == nullptr){
        return CAMSTATUS_PARAM_INVALID;
      }
        if(TYOpenDeviceWithIP(ip_address, &m_hDevice) != TY_STATUS_OK){
          return CAMSTATUS_ERROR;
        }
        assert(m_pCoordinateMapper == NULL);
        m_pCoordinateMapper = new CoordinateMapper(m_hDevice);
        return CAMSTATUS_SUCCESS;
    }
    
    CameraSourceStatus OpenDevice()
    {
        return OpenDevice(1);
    }
    
    void CloseDevice()
    {
        assert_ok(TYCloseDevice(m_hDevice));
        m_hDevice = 0;
    }
    
    CameraSourceStatus Config(const char *data)
    {
        return CAMSTATUS_NOTSUPPORT;
    }
    
    int GetProperty(int prop_id, void *data_buff, int size)
    {
        switch(prop_id){
            case PROP_DEVICE_INFO:{
                assert(size == sizeof(DeviceInfo));
                DeviceInfo* p = (DeviceInfo*) data_buff;
                p->hardware_model = 0;
                p->major = m_curDevInfo.hardwareVersion.major;
                p->minor = m_curDevInfo.hardwareVersion.minor;
                strncpy((char*)p->sn, m_curDevInfo.id, sizeof(p->sn));
                strncpy((char*)p->str_name, m_curDevInfo.modelName, sizeof(p->str_name));
                return sizeof(DeviceInfo);
            }
            case PROP_LASER_POW:{
                assert(size == sizeof(int));
                int32_t power;
                int err = TYGetInt(m_hDevice, TY_COMPONENT_LASER, TY_INT_LASER_POWER, &power);
                if(err != TY_STATUS_OK){
                    return CAMSTATUS_ERROR;
                }
                *(unsigned char*)data_buff = power;;
                return sizeof(char); // old interface is char
            }
            case PROP_DEPTH_RESOLUTION:{
                assert(size == sizeof(int));
                int32_t mode;
                assert_ok(TYGetEnum(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, &mode));
                int* out = (int*)data_buff;
                switch(mode){
                    case TY_IMAGE_MODE_320x240:
                        *out = RESO_MODE_320x240;
                        return sizeof(int);
                    case TY_IMAGE_MODE_640x480:
                        *out = RESO_MODE_640x480;
                        return sizeof(int);
                    case TY_IMAGE_MODE_1280x960:
                        *out = RESO_MODE_1280x960;
                        return sizeof(int);
                    case TY_IMAGE_MODE_2592x1944:
                        *out = RESO_MODE_2592x1944;
                        return sizeof(int);
                }
                return CAMSTATUS_NOTSUPPORT;
            }
            case PROP_CALIB_DEPTH_INTRINSIC:{
                assert(size == sizeof(CamIntrinsic));
                TY_CAMERA_INTRINSIC intr;
                int32_t w, h;
                assert_ok(TYGetStruct(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_STRUCT_CAM_INTRINSIC, &intr, sizeof(intr)));
                assert_ok(TYGetInt(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_INT_WIDTH, &w));
                assert_ok(TYGetInt(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_INT_HEIGHT, &h));
                CamIntrinsic* out = (CamIntrinsic*)data_buff;
                out->width = w;
                out->height = h;
                memcpy(out->data, intr.data, sizeof(out->data));
                return sizeof(CamIntrinsic);
            }
            case PROP_POINT_CLOUD_OUTPUT:{
                assert(size == sizeof(bool));
                bool* en = (bool*)data_buff;
                *en = (m_Point3DComponent == TY_COMPONENT_POINT3D_CAM);
                return sizeof(bool);
            }
            case PROP_WAIT_NEXTFRAME_TIMEOUT:{
                assert(size == sizeof(int));
                *(int*)data_buff = 0;
                return sizeof(int);
            }
            case PROP_UNDISTORT_IR:{
                assert(size == sizeof(bool));
                bool* en = (bool*)data_buff;
                assert_ok(TYGetBool(m_hDevice, TY_COMPONENT_IR_CAM_LEFT, TY_BOOL_UNDISTORTION, en));
                return sizeof(bool);
            }
            case PROP_COORDINATE_MAP:{
                assert(size == sizeof(void*));
                *(ICoordinateMapper**)data_buff = m_pCoordinateMapper;
                return sizeof(void*);
            }
            case PROP_TRIGGER_MODE:{
                assert(size == sizeof(bool));
                bool* en = (bool*)data_buff;
                assert_ok(TYGetBool(m_hDevice, TY_COMPONENT_DEVICE, TY_BOOL_TRIGGER_MODE, en));
                return sizeof(bool);
            }
            case PROP_AUTO_GAIN_CTRL:{
                assert(size == sizeof(CamGainCtrlParam));
                CamGainCtrlParam* out = (CamGainCtrlParam*)data_buff;
                bool auto_gain;
                assert_ok(TYGetBool(m_hDevice, CAM_DATA_TO_TY_COMPONENT(out->cam_id)
                            , TY_BOOL_AUTO_GAIN, &auto_gain));
                if(auto_gain){
                    out->gain = -1;
                } else {
                    assert_ok(TYGetInt(m_hDevice, CAM_DATA_TO_TY_COMPONENT(out->cam_id)
                            , TY_INT_GAIN, &out->gain));
                }
                return sizeof(CamGainCtrlParam);
            }
        }
        return CAMSTATUS_NOTSUPPORT;
    }
    
    int SetProperty(int prop_id, const void *data, int size)
    {
        switch(prop_id){
            case PROP_LASER_POW:{
                assert(size == sizeof(uint32_t));
                int power = *(uint32_t*)data;
                int err = TYSetInt(m_hDevice, TY_COMPONENT_LASER, TY_INT_LASER_POWER, power);
                if(err != TY_STATUS_OK){
                    return CAMSTATUS_NOTSUPPORT;
                }
                return CAMSTATUS_SUCCESS;
            }
            case PROP_WORKMODE:{
                assert(size == sizeof(int));
                int mode = *(int*)data;
                return setWorkMode(mode);
            }
            case PROP_DEPTH_RESOLUTION:{
                assert(size == sizeof(int));
                int mode = *(int*)data;
                switch(mode){
                    case RESO_MODE_320x240:
                        assert_ok(TYSetEnum(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_320x240));
                        return CAMSTATUS_SUCCESS;
                    case RESO_MODE_640x480:
                        assert_ok(TYSetEnum(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_640x480));
                        return CAMSTATUS_SUCCESS;
                    case RESO_MODE_1280x960:
                        assert_ok(TYSetEnum(m_hDevice, TY_COMPONENT_DEPTH_CAM, TY_ENUM_IMAGE_MODE, TY_IMAGE_MODE_1280x960));
                        return CAMSTATUS_SUCCESS;
                    default:
                        break;
                }
                return CAMSTATUS_NOTSUPPORT;
            }
            case PROP_POINT_CLOUD_OUTPUT:{
                assert(size == sizeof(bool));
                bool en = *(bool*)data;
                if(en){
                    m_Point3DComponent = TY_COMPONENT_POINT3D_CAM;
                } else {
                    m_Point3DComponent = 0;
                }
                return CAMSTATUS_SUCCESS;
            }
            case PROP_FRAME_READY_CALLBACK:{
                assert(size == sizeof(void*));
                m_callback = (EventCallbackFunc)data;
                return CAMSTATUS_SUCCESS;
            }
            case PROP_CALLBACK_USER_DATA:{
                assert(size == sizeof(void*));
                m_userdata = (void*)data;
                return CAMSTATUS_SUCCESS;
            }
            case PROP_UNDISTORT_IR:{
                assert(size == sizeof(bool));
                bool en = *(bool*)data;
                assert_ok(TYSetBool(m_hDevice, TY_COMPONENT_IR_CAM_LEFT, TY_BOOL_UNDISTORTION, en));
                assert_ok(TYSetBool(m_hDevice, TY_COMPONENT_IR_CAM_RIGHT, TY_BOOL_UNDISTORTION, en));
                return CAMSTATUS_SUCCESS;
            }
            case PROP_TRIGGER_MODE:{
                assert(size == sizeof(bool));
                bool trigger = *(bool*)data;
                assert_ok(TYSetBool(m_hDevice, TY_COMPONENT_DEVICE, TY_BOOL_TRIGGER_MODE, trigger));
                return CAMSTATUS_SUCCESS;
            }
            case PROP_AUTO_GAIN_CTRL:{
                assert(size == sizeof(CamGainCtrlParam));
                CamGainCtrlParam* param = (CamGainCtrlParam*)data;
                if(param->gain == -1){
                    assert_ok(TYSetBool(m_hDevice, CAM_DATA_TO_TY_COMPONENT(param->cam_id)
                            , TY_BOOL_AUTO_GAIN, true));
                } else {
                    assert_ok(TYSetInt(m_hDevice, CAM_DATA_TO_TY_COMPONENT(param->cam_id)
                            , TY_INT_GAIN, param->gain));
                }
                return CAMSTATUS_SUCCESS;
            }
        }
        return CAMSTATUS_NOTSUPPORT;
    }
    
    int setWorkMode(int mode)
    {
        if(mode == WORKMODE_IDLE){
            TYStopCapture(m_hDevice);
            return CAMSTATUS_SUCCESS;
        }
    
        int enabledComponents = 0;
        if(mode & WORKMODE_IR){
            enabledComponents |= TY_COMPONENT_IR_CAM_LEFT | TY_COMPONENT_IR_CAM_RIGHT;
        }
        if(mode & WORKMODE_DEPTH){
            enabledComponents |= TY_COMPONENT_DEPTH_CAM | m_Point3DComponent;
        }
        if(mode & WORKMODE_RGB){
            enabledComponents |= TY_COMPONENT_RGB_CAM;
        }
    
        // Start capture
        assert_ok(TYEnableComponents(m_hDevice, enabledComponents));
        int32_t bufferSize;
        assert_ok(TYGetFrameBufferSize(m_hDevice, &bufferSize));
        assert(bufferSize > 0);
        m_fb0.resize(bufferSize);
        m_fb1.resize(bufferSize);
        assert_ok(TYEnqueueBuffer(m_hDevice, &m_fb0[0], m_fb0.size()));
        assert_ok(TYEnqueueBuffer(m_hDevice, &m_fb1[0], m_fb1.size()));
    
        if(m_callback){
            assert_ok(TYRegisterCallback(m_hDevice, _frameCb, this));
        }
    
        assert_ok(TYStartCapture(m_hDevice));
        return CAMSTATUS_SUCCESS;
    }

    private:
    
    static void _frameCb(TY_FRAME_DATA* frame, void* userdata)
    {
        CameraVideoSourceWrapper* p = (CameraVideoSourceWrapper*)userdata;
        if(p->m_callback){
            p->m_frame = *frame;
            p->m_callback(p->m_userdata);
        }
        assert_ok(TYEnqueueBuffer(p->m_hDevice, frame->userBuffer, frame->bufferSize));
    }
    
    void updateCamList()
    {
        int32_t n;
        TY_STATUS ss= TYGetDeviceNumber(&n);
        if (ss == TY_STATUS_NOT_INITED){
            printf("***** library not inited ,call TYInitLib first ****\n");
            printf("***** call TYDeInitLib before exit ****\n");
            assert(0);
            return;
        }
        if (n != 0){
          m_devInfos.resize(n);
        }
        else{
          m_devInfos.clear();
          return;
        }
        assert_ok(TYGetDeviceList(&m_devInfos[0], m_devInfos.size(), &n));
    }
    
};


} // namespace

//////////////////////////////////////////////////////////////////
namespace percipio {

static ICameraVideoSource* CreateSource(HardwareModel model)
{
    return new CameraVideoSourceWrapper;
}

static void ReleaseSource(ICameraVideoSource* ptr)
{
    delete ptr;
}

static void SetLogLevel(int level)
{
    //not used now
}

static int LibVersion()
{
    return PERCIPIO_CAMPORT_LIB_BUILD_VERSION;
}


}

#endif

