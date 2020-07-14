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

Camera::Camera(QWidget *parent) : QMainWindow(parent), ui(new Ui::Camera), m_camera(0), m_imageSize(100),
	m_imageCapture(0), m_mediaRecorder(0), isCapturingImage(false), applicationExiting(false)
{
    ui->setupUi(this);

    // Camera devices:
    QActionGroup *videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);

    // The QCameraInfo class provides general information about camera devices.
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        QAction *videoDeviceAction = new QAction(cameraInfo.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraInfo));
        if (cameraInfo == QCameraInfo::defaultCamera())
            videoDeviceAction->setChecked(true);
        ui->menuDevices->addAction(videoDeviceAction);
    }

    connect(videoDevicesGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateCameraDevice(QAction*)));
	connect(ui->captureWidget, SIGNAL(currentChanged(int)), this, SLOT(updateCaptureMode(int)));

    setCamera(QCameraInfo::defaultCamera());
}

int Camera::InitRecorder()
{
	return 0;
}

int Camera::UnInitRecorder()
{
	if (NULL != m_mediaRecorder) {
		delete m_mediaRecorder;
		m_mediaRecorder = NULL;
	}
	if (NULL != m_imageCapture) {
		delete m_imageCapture;
		m_imageCapture = NULL;
	}
	if (NULL != m_camera) {
		delete m_camera;
		m_camera = NULL;
	}
	return 0;
}

Camera::~Camera()
{
    TRACE(EN_CTRACE_INFO,"Camera");
	UnInitRecorder();
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

bool Camera::DisplayImage(const QImage &image)
{
	int width = ui->label->width() * m_imageSize / 100;
	int height = ui->label->height() * m_imageSize / 100;
	QImage imgScaled = image.scaled(width, height, Qt::KeepAspectRatio);
	
	QMatrix matrix;
	matrix.rotate(180);
	QImage imgRotate = imgScaled.transformed(matrix);

	ui->label->setPixmap(QPixmap::fromImage(imgRotate));
	return true;
}

bool Camera::setImageValue(int size)
{
	m_imageSize = size;
	return true;
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
	qDebug() << cloneFrame.mappedBytes();
	DisplayImage(image);
	QByteArray buffer;
	bool ret = image.loadFromData(buffer, "png");
    cloneFrame.unmap();
	return true;
}

void Camera::setCameraSurface(VideoWidgetSurface*& videoSurface)
{
	// 6.图像连续缩放可以利用Horizontal Slider，该部件指向的值为整型value
	connect(videoSurface, SIGNAL(CaptureFrame(const QVideoFrame)), this, SLOT(sendVideo(const QVideoFrame)));
	ui->exposureCompensation->setMinimum(1);
	ui->exposureCompensation->setMinimum(100);
	connect(ui->exposureCompensation, SIGNAL(valueChanged(int)), this, SLOT(setImageValue(int)));
	videoSurface->setSource(m_camera);
	m_surface = videoSurface;
}

void Camera::setCameraAndImageCapture(QCameraImageCapture*& imageCapture)
{
	//The QCameraImageCapture class is used for the recording of media content.
	connect(imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
	connect(imageCapture, SIGNAL(imageCaptured(int, QImage)), this, SLOT(processCapturedImage(int, QImage)));
	connect(imageCapture, SIGNAL(imageSaved(int, QString)), this, SLOT(imageSaved(int, QString)));
	connect(imageCapture, SIGNAL(error(int, QCameraImageCapture::Error, QString)),
		this, SLOT(displayCaptureError(int, QCameraImageCapture::Error, QString)));
	m_imageCapture = imageCapture;
}

void Camera::setCameraAndMediaRecorder(QMediaRecorder*& mediaRecorder)
{
    connect(mediaRecorder, SIGNAL(stateChanged(QMediaRecorder::State)), this, SLOT(updateRecorderState(QMediaRecorder::State)));
    connect(mediaRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateRecordTime()));
    connect(mediaRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(displayRecorderError()));
    mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));
    updateRecorderState(mediaRecorder->state());
	m_mediaRecorder = mediaRecorder;

}

void Camera::setCameraSelf(QCamera*& camera)
{
	connect(camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(updateCameraState(QCamera::State)));
	connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));
	connect(camera, SIGNAL(lockStatusChanged(QCamera::LockStatus, QCamera::LockChangeReason)),
		this, SLOT(updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason)));
	camera->setViewfinder(ui->viewfinder);
	updateCameraState(camera->state());
    updateLockStatus(camera->lockStatus(), QCamera::UserRequest);
	m_camera = camera;
}

void Camera::setCamera(const QCameraInfo &cameraInfo)
{
	UnInitRecorder();
	QCamera *camera = new QCamera(cameraInfo);
	QMediaRecorder* memdiaRecorder = new QMediaRecorder(camera);
	QCameraImageCapture* imageCapture = new QCameraImageCapture(camera);
	// VideoWidgetSurface* videoSurface = new VideoWidgetSurface();
	setCameraSelf(camera);
	setCameraAndImageCapture(imageCapture);
	setCameraAndMediaRecorder(memdiaRecorder);
	// setCameraSurface(videoSurface);
    connect(ui->exposureCompensation, SIGNAL(valueChanged(int)), this, SLOT(setExposureCompensation(int)));
    ui->captureWidget->setTabEnabled(0, (camera->isCaptureModeSupported(QCamera::CaptureStillImage)));
    ui->captureWidget->setTabEnabled(1, (camera->isCaptureModeSupported(QCamera::CaptureVideo)));
	// ui->captureWidget->installEventFilter(this);
    updateCaptureMode(1);

    camera->start();
}

void Camera::keyPressEvent(QKeyEvent * event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_CameraFocus:
        displayViewfinder();
		m_camera->searchAndLock();
        event->accept();
        break;
    case Qt::Key_Camera:
        if (m_camera->captureMode() == QCamera::CaptureStillImage) {
            takeImage();
        } else {
            if (m_mediaRecorder->state() == QMediaRecorder::RecordingState)
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
		m_camera->unlock();
        break;
    default:
        QMainWindow::keyReleaseEvent(event);
    }
}

void Camera::updateRecordTime()
{
    QString str = QString("Recorded %1 sec").arg(m_mediaRecorder->duration()/1000);
    ui->statusbar->showMessage(str);
}

void Camera::processCapturedImage(int requestId, const QImage& img)
{
    Q_UNUSED(requestId);
    QImage scaledImage = img.scaled(ui->viewfinder->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->lastImagePreviewLabel->setPixmap(QPixmap::fromImage(scaledImage));

    // Display captured image for 4 seconds.
    displayCapturedImage();
    QTimer::singleShot(4000, this, SLOT(displayViewfinder()));
}

void Camera::configureCaptureSettings()
{
    switch (m_camera->captureMode()) {
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
    VideoSettings settingsDialog(m_mediaRecorder);

    settingsDialog.setAudioSettings(audioSettings);
    settingsDialog.setVideoSettings(videoSettings);
    settingsDialog.setFormat(videoContainerFormat);

    if (settingsDialog.exec()) {
        audioSettings = settingsDialog.audioSettings();
        videoSettings = settingsDialog.videoSettings();
        videoContainerFormat = settingsDialog.format();

		m_mediaRecorder->setEncodingSettings(audioSettings, videoSettings, videoContainerFormat);
    }
}

void Camera::configureImageSettings()
{
    ImageSettings settingsDialog(m_imageCapture);

    settingsDialog.setImageSettings(imageSettings);

    if (settingsDialog.exec()) {
        imageSettings = settingsDialog.imageSettings();
		m_imageCapture->setEncodingSettings(imageSettings);
    }
}

void Camera::record()
{
	m_mediaRecorder->record();
    updateRecordTime();
}

void Camera::pause()
{
	m_mediaRecorder->pause();
}

void Camera::stop()
{
	m_mediaRecorder->stop();
}

void Camera::setMuted(bool muted)
{
	m_mediaRecorder->setMuted(muted);
}

void Camera::toggleLock()
{
    switch (m_camera->lockStatus()) {
    case QCamera::Searching:
    case QCamera::Locked:
		m_camera->unlock();
        break;
    case QCamera::Unlocked:
		m_camera->searchAndLock();
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
	m_imageCapture->capture();
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
	m_camera->start();
}

void Camera::stopCamera()
{
	m_camera->stop();
}

void Camera::updateCaptureMode(int index)
{
    int tabIndex = ui->captureWidget->currentIndex();
    QCamera::CaptureModes captureMode = tabIndex == 0 ? QCamera::CaptureStillImage : QCamera::CaptureVideo;

    if (m_camera->isCaptureModeSupported(captureMode))
		m_camera->setCaptureMode(captureMode);
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
		break;
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
	m_camera->exposure()->setExposureCompensation(index*0.5);
}

void Camera::displayRecorderError()
{
    QMessageBox::warning(this, tr("Capture error"), m_mediaRecorder->errorString());
}

void Camera::displayCameraError()
{
    QMessageBox::warning(this, tr("Camera error"), m_camera->errorString());
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
}