#ifndef VIDEOWIDGETSURFACE_H
#define VIDEOWIDGETSURFACE_H
#include <QAbstractVideoSurface>
#include <QVideoProbe>
#include <QCamera>
#include <QVideoFrame>
class VideoWidgetSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    VideoWidgetSurface(QObject *parent = 0);
    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

    bool setSource(QCamera *pCamera);
private slots:
	virtual bool present(const QVideoFrame &frame);

signals:
    void CaptureFrame(const QVideoFrame &frame);

private:
     QVideoProbe m_Probe;/*android下,目前只能用probe捕获视频*/
};

#endif // VIDEOWIDGETSURFACE_H
