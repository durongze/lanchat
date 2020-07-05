/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include <QCamera>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QMainWindow>
#include <QVideoRendererControl>
#include <QVideoSurfaceFormat>
#include "videowidgetsurface.h"
QT_BEGIN_NAMESPACE
namespace Ui { class Camera; }
QT_END_NAMESPACE

class Camera : public QMainWindow
{
    Q_OBJECT

private:
    Camera(QWidget *parent = 0);
public:
    ~Camera();
    static Camera* getCamera();
    static void delCamera();
private slots:
    bool setImageValue(int size);
    bool DisplayImage(const QImage &image);
    bool sendVideo(const QVideoFrame &frame);
    void setCamera(const QCameraInfo &cameraInfo);
	void setCameraSelf(QCamera*& camera);
	void setCameraSurface(VideoWidgetSurface*& videoSurface);
	void setCameraAndImageCapture(QCameraImageCapture*& imageCapture);
	void setCameraAndMediaRecorder(QMediaRecorder*& mediaRecorder); 
    void startCamera();
    void stopCamera();

    void record();
    void pause();
    void stop();
    void setMuted(bool);

    void toggleLock();
    void takeImage();
    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);

    void configureCaptureSettings();
    void configureVideoSettings();
    void configureImageSettings();

    void displayRecorderError();
    void displayCameraError();

    void updateCameraDevice(QAction *action);

    void updateCameraState(QCamera::State);
    void updateCaptureMode(int index);
    void updateRecorderState(QMediaRecorder::State state);
    void setExposureCompensation(int index);

    void updateRecordTime();

    void processCapturedImage(int requestId, const QImage &img);
    void updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason);

    void displayViewfinder();
    void displayCapturedImage();

    void readyForCapture(bool ready);
    void imageSaved(int id, const QString &fileName);

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
private:
	int InitRecorder();
	int UnInitRecorder();
private:
    Ui::Camera *ui;
    /* QVideoWidget搭配使用用来显示摄像机视频 */
    QCamera *m_camera;
    /* QCameraimageCapture可以抓拍摄像机图像 */
    QCameraImageCapture *m_imageCapture;
    /* QMediaRecoder可以保存摄像机视频 */
    QMediaRecorder* m_mediaRecorder;

    QImageEncoderSettings imageSettings;
    QAudioEncoderSettings audioSettings;
    QVideoEncoderSettings videoSettings;
    QString videoContainerFormat;
    bool isCapturingImage;
    bool applicationExiting;

    /* QVideoFrame 继承了QAbstractVideoSurface:
    (1)重载虚基函数QList<QVideoFrame::PixelFormat>supportedPixelFormats(QAbstractVideoBuffer::HandleTypehandleType=QAbstractVideoBuffer::NoHandle)const;用于设置程序支持的原始视频格式;
    (2)重载基类的bool present(constQVideoFrame&frame) 函数，用于获取当前帧的视频源数据;
    (3)重载基类的bool start(constQVideoSurfaceFormat&format) 函数，用于启动视频表面及进行相应的窗口设置;
    (4)重载基类的void stop() 函数，用于停止当前的视频表面和释放调用start时所产生的资源.
    (5)添加void paint(QPainter*painter) 函数,用以绘制视频图像,以在本地窗口中输出.
        其次, 新建一个QCamera对象m_pCamera, 新建一个VideoWidgetSurface对象m_pVideoSurface;
        通过 m_pCamera->service()->requestControl<QVideoRendererControl*>() 方法获取 QVideoRendererControl 的指针pControl,
        如果该指针存在,则调用pControl 的 setSurface 关联到 m_pVideoSurface,
        这样当m_pCamera执行完Start()后,就可以在m_pVideoSuface对象的present函数中得到每一帧的QVideoFrame视频数据.
        在测试程序中获取的视频数据格式为RGB32,需要先转为YUV240P的格式,然后再用FFmpeg的H.264编码功能实现视频数据编码.*/
    VideoWidgetSurface *m_surface;
	int m_imageSize;
    QVideoSurfaceFormat  *m_format;
    static Camera* instanceCamera;
};

#endif
