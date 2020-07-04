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
#include "trace.h"
#include "camera.h"
#include "ui_camera.h"
#include "videosettings.h"
#include "imagesettings.h"
#include "trace.h"
#include <QMediaService>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QMediaMetaData>

#include <QMessageBox>
#include <QPalette>

#include <QtWidgets>

Q_DECLARE_METATYPE(QCameraInfo)

Camera* Camera::instanceCamera = NULL;

/* 通过发信号告诉主程序建立本地视频服务器, 借鉴传输文件 */
Camera::Camera(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Camera),
    camera(0),
    imageCapture(0),
    mediaRecorder(0),
    isCapturingImage(false),
    applicationExiting(false)
{
    ui->setupUi(this);

    // Camera devices:
    QActionGroup *videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);

    // The QCameraInfo class provides general information about camera devices.
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {

        // The QAction class provides an abstract user interface action that can be inserted into widgets.
        QAction *videoDeviceAction = new QAction(cameraInfo.description(), videoDevicesGroup);

        videoDeviceAction->setCheckable(true);

        // The QVariant class acts like a union for the most common Qt data types.
        videoDeviceAction->setData(QVariant::fromValue(cameraInfo));

        if (cameraInfo == QCameraInfo::defaultCamera())
            videoDeviceAction->setChecked(true);

        ui->menuDevices->addAction(videoDeviceAction);
    }


    connect(videoDevicesGroup, SIGNAL(triggered(QAction*)), SLOT(updateCameraDevice(QAction*)));
    connect(ui->captureWidget, SIGNAL(currentChanged(int)), SLOT(updateCaptureMode()));

    setCamera(QCameraInfo::defaultCamera());
}

Camera::~Camera()
{
    TRACE(EN_CTRACE_INFO,"Camera");
    delete mediaRecorder;
    delete imageCapture;
    delete camera;
}

Camera* Camera::getCamera()
{
    if (NULL == Camera::instanceCamera)
        instanceCamera = new Camera;
    return instanceCamera;
}

void Camera::delCamera()
{
    if (NULL  !=  Camera::instanceCamera)
        delete instanceCamera;
    instanceCamera = NULL;
}

/* 发送视频到远端 */
bool Camera::sendVideo(const QVideoFrame &frame)
{
    Q_UNUSED(frame);
	if (!frame.isValid()) {
		return false;
	}
    QVideoFrame cloneFrame(frame);
    cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
	const QImage::Format& format = QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat());
    QImage image(cloneFrame.bits(), cloneFrame.width(), cloneFrame.height(), format);
    ui->label->setPixmap(QPixmap::fromImage(image));
    qDebug()<<cloneFrame.mappedBytes();
    cloneFrame.unmap();
	QByteArray buffer;
	bool ret = image.loadFromData(buffer, "png");

	return true;
    /*4. 图像缩放采用scaled函数。scaled函数中width和height表示缩放后图像的宽和高，即将原图像缩放到(width,height)大小。*/
	QImage* imgScaled = new QImage;
    *imgScaled = image.scaled(ui->label->width, ui->label->height, Qt::KeepAspectRatio);
    ui->label->setPixmap(QPixmap::fromImage(*imgScaled));
    /*5. 图像顺时针旋转利用QImage类的transformed函数，向transformed函数传入QMatrix对象，QMatrix对象指定了旋转的角度。*/
	QImage* imgRotate = new QImage;
	QMatrix matrix;
	matrix.rotate(270);
	*imgRotate = image.transformed(matrix);
	ui->label->setPixmap(QPixmap::fromImage(*imgRotate));
    /*6. 图像连续缩放可以利用Horizontal Slider，该部件指向的值为整型value。*/
	int value = 40;
	image.scaled(ui->label->width * value / 100, ui->label->height * value / 100, Qt::KeepAspectRatio);
	return true;
}

void Camera::setCamera(const QCameraInfo &cameraInfo)
{
    delete imageCapture;
    delete mediaRecorder;
    delete camera;
	QCamera *c = NULL;

    camera = new QCamera(cameraInfo);
    c = camera;
    connect(c, SIGNAL(stateChanged(QCamera::State)), this, SLOT(updateCameraState(QCamera::State)));
    connect(c, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));

    mediaRecorder = new QMediaRecorder(c);
    connect(mediaRecorder, SIGNAL(stateChanged(QMediaRecorder::State)), this, SLOT(updateRecorderState(QMediaRecorder::State)));

    imageCapture = new QCameraImageCapture(c);

    connect(mediaRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateRecordTime()));
    connect(mediaRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(displayRecorderError()));

    mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));

    connect(ui->exposureCompensation, SIGNAL(valueChanged(int)), SLOT(setExposureCompensation(int)));
#if 0
    c->setViewfinder(ui->viewfinder);
#else
    c->setViewfinder(&m_surface);
#endif
    updateCameraState(c->state());
    updateLockStatus(c->lockStatus(), QCamera::UserRequest);
    updateRecorderState(mediaRecorder->state());
    //信号和槽的链接要注意参数部分，不能有变量名
    connect(&m_surface, SIGNAL(CaptureFrame(const QVideoFrame)), this, SLOT(sendVideo(const QVideoFrame)));

    //The QCameraImageCapture class is used for the recording of media content.
    connect(imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
    connect(imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
    connect(imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
    connect(imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this,
            SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));

    connect(c, SIGNAL(lockStatusChanged(QCamera::LockStatus,QCamera::LockChangeReason)),
            this, SLOT(updateLockStatus(QCamera::LockStatus,QCamera::LockChangeReason)));

    ui->captureWidget->setTabEnabled(0, (c->isCaptureModeSupported(QCamera::CaptureStillImage)));
    ui->captureWidget->setTabEnabled(1, (c->isCaptureModeSupported(QCamera::CaptureVideo)));

    updateCaptureMode();
    c->start();
}

void Camera::keyPressEvent(QKeyEvent * event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        displayViewfinder();
        camera->searchAndLock();
        event->accept();
        break;
    case Qt::Key_Camera:
        if (camera->captureMode() == QCamera::CaptureStillImage) {
            takeImage();
        } else {
            if (mediaRecorder->state() == QMediaRecorder::RecordingState)
                stop();
            else
                record();
        }
        event->accept();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void Camera::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        camera->unlock();
        break;
    default:
        QMainWindow::keyReleaseEvent(event);
    }
}

void Camera::updateRecordTime()
{
    QString str = QString("Recorded %1 sec").arg(mediaRecorder->duration()/1000);
    ui->statusbar->showMessage(str);
}

void Camera::processCapturedImage(int requestId, const QImage& img)
{
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->viewfinder->size(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

    // Display captured image for 4 seconds.
    displayCapturedImage();
    QTimer::singleShot(4000, this, SLOT(displayViewfinder()));
}

void Camera::configureCaptureSettings()
{
    switch (camera->captureMode()) {
    case QCamera::CaptureStillImage:
        configureImageSettings();
        break;
    case QCamera::CaptureVideo:
        configureVideoSettings();
        break;
    default:
        break;
    }
}

void Camera::configureVideoSettings()
{
    VideoSettings settingsDialog(mediaRecorder);

    settingsDialog.setAudioSettings(audioSettings);
    settingsDialog.setVideoSettings(videoSettings);
    settingsDialog.setFormat(videoContainerFormat);

    if (settingsDialog.exec()) {
        audioSettings = settingsDialog.audioSettings();
        videoSettings = settingsDialog.videoSettings();
        videoContainerFormat = settingsDialog.format();

        mediaRecorder->setEncodingSettings(
                    audioSettings,
                    videoSettings,
                    videoContainerFormat);
    }
}

void Camera::configureImageSettings()
{
    ImageSettings settingsDialog(imageCapture);

    settingsDialog.setImageSettings(imageSettings);

    if (settingsDialog.exec()) {
        imageSettings = settingsDialog.imageSettings();
        imageCapture->setEncodingSettings(imageSettings);
    }
}

void Camera::record()
{
    mediaRecorder->record();
    updateRecordTime();
}

void Camera::pause()
{
    mediaRecorder->pause();
}

void Camera::stop()
{
    mediaRecorder->stop();
}

void Camera::setMuted(bool muted)
{
    mediaRecorder->setMuted(muted);
}

void Camera::toggleLock()
{
    switch (camera->lockStatus()) {
    case QCamera::Searching:
    case QCamera::Locked:
        camera->unlock();
        break;
    case QCamera::Unlocked:
        camera->searchAndLock();
    }
}

void Camera::updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason)
{
    QColor indicationColor = Qt::black;

    switch (status) {
    case QCamera::Searching:
        indicationColor = Qt::yellow;
        ui->statusbar->showMessage(tr("Focusing..."));
        ui->lockButton->setText(tr("Focusing..."));
        break;
    case QCamera::Locked:
        indicationColor = Qt::darkGreen;
        ui->lockButton->setText(tr("Unlock"));
        ui->statusbar->showMessage(tr("Focused"), 2000);
        break;
    case QCamera::Unlocked:
        indicationColor = reason == QCamera::LockFailed ? Qt::red : Qt::black;
        ui->lockButton->setText(tr("Focus"));
        if (reason == QCamera::LockFailed)
            ui->statusbar->showMessage(tr("Focus Failed"), 2000);
    }

    QPalette palette = ui->lockButton->palette();
    palette.setColor(QPalette::ButtonText, indicationColor);
    ui->lockButton->setPalette(palette);
}

void Camera::takeImage()
{
    isCapturingImage = true;
    imageCapture->capture();
}

void Camera::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    isCapturingImage = false;
}

void Camera::startCamera()
{
    camera->start();
}

void Camera::stopCamera()
{
    camera->stop();
}

void Camera::updateCaptureMode()
{
    int tabIndex = ui->captureWidget->currentIndex();
    QCamera::CaptureModes captureMode = tabIndex == 0 ? QCamera::CaptureStillImage : QCamera::CaptureVideo;

    if (camera->isCaptureModeSupported(captureMode))
        camera->setCaptureMode(captureMode);
}

void Camera::updateCameraState(QCamera::State state)
{
    switch (state) {
    case QCamera::ActiveState:
        ui->actionStartCamera->setEnabled(false);
        ui->actionStopCamera->setEnabled(true);
        ui->captureWidget->setEnabled(true);
        ui->actionSettings->setEnabled(true);
        break;
    case QCamera::UnloadedState:
    case QCamera::LoadedState:
        ui->actionStartCamera->setEnabled(true);
        ui->actionStopCamera->setEnabled(false);
        ui->captureWidget->setEnabled(false);
        ui->actionSettings->setEnabled(false);
    }
}

void Camera::updateRecorderState(QMediaRecorder::State state)
{
    switch (state) {
    case QMediaRecorder::StoppedState:
        ui->recordButton->setEnabled(true);
        ui->pauseButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        break;
    case QMediaRecorder::PausedState:
        ui->recordButton->setEnabled(true);
        ui->pauseButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        break;
    case QMediaRecorder::RecordingState:
        ui->recordButton->setEnabled(false);
        ui->pauseButton->setEnabled(true);
        ui->stopButton->setEnabled(true);
        break;
    }
}

void Camera::setExposureCompensation(int index)
{
    camera->exposure()->setExposureCompensation(index*0.5);
}

void Camera::displayRecorderError()
{
    QMessageBox::warning(this, tr("Capture error"), mediaRecorder->errorString());
}

void Camera::displayCameraError()
{
    QMessageBox::warning(this, tr("Camera error"), camera->errorString());
}

void Camera::updateCameraDevice(QAction *action)
{
    setCamera(qvariant_cast<QCameraInfo>(action->data()));
    //将视频数据发送到远端
}

void Camera::displayViewfinder()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Camera::displayCapturedImage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void Camera::readyForCapture(bool ready)
{
    ui->takeImageButton->setEnabled(ready);
}

void Camera::imageSaved(int id, const QString &fileName)
{
    Q_UNUSED(id);
    Q_UNUSED(fileName);

    isCapturingImage = false;
    if (applicationExiting)
        close();
}

void Camera::closeEvent(QCloseEvent *event)
{
    TRACE(EN_CTRACE_INFO,"Camera");
    if (isCapturingImage) {
        setEnabled(false);
        applicationExiting = true;
        event->ignore();
    } else {
        event->accept();
    }
    delCamera();
}
