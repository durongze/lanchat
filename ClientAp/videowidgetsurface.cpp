#include "videowidgetsurface.h"
#include "trace.h"

VideoWidgetSurface::VideoWidgetSurface(QObject *parent) :
    QAbstractVideoSurface(parent)
{
}

QList<QVideoFrame::PixelFormat> VideoWidgetSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    QList<QVideoFrame::PixelFormat> lst;

    /*lst.push_back(QVideoFrame::Format_YUYV);//Qt现在不支持此格式，因为Qt内部用了QImage来处理视频帧*/
    lst.push_back(QVideoFrame::Format_RGB32);
    lst.push_back(QVideoFrame::Format_BGR32);

    return lst;
}

/* 捕获视频帧.windows下格式是RGB32;android下是NV21 */
bool VideoWidgetSurface::present(const QVideoFrame &frame)
{
    // qDebug("CCaptureVideoFrame::present format:%d", frame.pixelFormat());
    emit CaptureFrame(frame);
    return true;
}

/* 根据不同的平台,设置捕获方式, windows下,只能用下面方式捕获视频 */
bool VideoWidgetSurface::setSource(QCamera *pCamera)
{
    bool ret = true;
#ifdef ANDROID
    //android下,目前只能用probe捕获视频
    ret = m_Probe.setSource(pCamera);
    if(ret)
    {
        connect(&m_Probe, SIGNAL(videoFrameProbed(QVideoFrame)),
                          SLOT(present(QVideoFrame)));
    }
#else
    pCamera->setViewfinder(this);
#endif
    return ret;
}
