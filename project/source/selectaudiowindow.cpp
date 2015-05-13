#include "selectaudiowindow.h"
#include "ui_selectaudiowindow.h"

#include <QStandardPaths>
#include <QMessageBox>

#include "tooldefine.h"
#include "resourceobject.h"
using namespace ORION;


SelectAudioWindow::SelectAudioWindow(QWidget *parent) :
QDialog(parent), audioHash_(0), player_(NULL),
    ui(new Ui::SelectAudioWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	initFromDb();

}

SelectAudioWindow::~SelectAudioWindow()
{
	delete player_;
    delete ui;
}


void  SelectAudioWindow::on_cancelpushButton_clicked(){

	reject();
}

void  SelectAudioWindow::on_savepushButton_clicked(){
	if (audioHash_ == 0){

		QListWidgetItem* item = ui->listWidget->currentItem();
		if (item == NULL){
			QMessageBox::warning(this, windowTitle(), UDQ_TR("��ѡ��һ����Ƶ��Դ!"));
			return;
		}
		audioHash_ = item->data(ITEM_HASHKEY).toUInt();
	}

	accept();
}


void SelectAudioWindow::initFromDb(void){

	// �����б�
	for (TResourceObjectIterator<AudioResourceObject> it; it; ++it){
		AudioResourceObject* audio = (AudioResourceObject*)(*it);

		if (audio->flags() & URF_TagGarbage)
			continue;

		QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
		item->setIcon(QIcon(UDQ_T(":/images/gallery_audio.png")));
		item->setText(audio->objectName());
		item->setData(ITEM_HASHKEY, audio->hashKey());

	}

}

void SelectAudioWindow::on_playpushButton_clicked(){
	Q_ASSERT(player_ != NULL);

	player_->setVolume(ui->volumehorizontalSlider->value());
	player_->play();

	ui->playpushButton->setEnabled(false);
	ui->pausepushButton->setEnabled(true);
	ui->stoppushButton->setEnabled(true);
}


void SelectAudioWindow::on_pausepushButton_clicked(){
	Q_ASSERT(player_);

	player_->pause();

	ui->playpushButton->setEnabled(true);
	ui->pausepushButton->setEnabled(false);
	ui->stoppushButton->setEnabled(true);
}
void SelectAudioWindow::on_stoppushButton_clicked(){
	Q_ASSERT(player_);

	player_->stop();

	ui->playpushButton->setEnabled(true);
	ui->pausepushButton->setEnabled(false);
	ui->stoppushButton->setEnabled(false);
}

void SelectAudioWindow::on_volumehorizontalSlider_valueChanged(int value){
	ui->volumelabel->setText(QString(UDQ_T("%1")).arg(value));

	if (player_){
		player_->setVolume(value);
	}
}

void SelectAudioWindow::handleError(QMediaPlayer::Error error){

	bool fault = true;
	switch (error)
	{
	case QMediaPlayer::NoError:
	{
		fault = false;
		ui->loglabel->setText(UDQ_TR("��Դ���ҵ�!"));
	}
		break;
	case QMediaPlayer::ResourceError:
	{
		ui->loglabel->setText(UDQ_TR("url��ַ�޷�����!"));
	}
		break;
	case QMediaPlayer::FormatError:
	{
		ui->loglabel->setText(UDQ_TR("�����ļ���ʽ�����޷�����!"));
	}
		break;
	case QMediaPlayer::NetworkError:
	{
		ui->loglabel->setText(UDQ_TR("������ϣ��޷�����Դ!"));
	}
		break;
	case QMediaPlayer::AccessDeniedError:
	{
		ui->loglabel->setText(UDQ_TR("��Դ�ܾ�����!"));
	}
		break;
	case QMediaPlayer::ServiceMissingError:
	{
		ui->loglabel->setText(UDQ_TR("û���ҵ������豸���޷�����!"));
	}
		break;
	default:
	{
		ui->loglabel->setText(UDQ_T("δ֪����!"));
	}
		break;
	}

	if (fault){
		ui->playpushButton->setEnabled(false);
		ui->pausepushButton->setEnabled(false);
		ui->stoppushButton->setEnabled(false);
	}
}

void  SelectAudioWindow::on_listWidget_itemDoubleClicked(QListWidgetItem * item){
	Q_ASSERT(item != NULL);

	if (player_ != NULL){
		player_->stop();
		delete player_;
		player_ = NULL;
	}


	audioHash_= item->data(ITEM_HASHKEY).toUInt();
	DEFINE_RESOURCE_HASHKEY_VALID(AudioResourceObject, audio, audioHash_);

	QString tempPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).at(0);
	// ����ǰ��Ƶ������ʱ�ļ�
	QString tempFIlePath = QString(UDQ_T("%1/audio_%2.mp3")).arg(tempPath).arg(audio->hashKey());
	
	// ȷ����ʱĿ¼����
	QDir dir(tempPath);
	if (!dir.exists()){
		dir.mkpath(tempPath);
	}

	QFile file(tempFIlePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("ָ������Ƶ��Դ����ʧ��!"));
		return;
	}
	file.write(audio->content_);
	file.close();

	player_ = new QMediaPlayer;
	connect(player_, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(handleError(QMediaPlayer::Error)));
	player_->setMedia(QUrl::fromLocalFile(tempFIlePath));

	// �����ļ�����
	on_playpushButton_clicked();
}