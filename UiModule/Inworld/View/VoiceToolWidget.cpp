/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   VoiceToolWidget.cpp
 *  @brief  Widget for voice communication control
 *          
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "VoiceToolWidget.h"
#include "CommunicationsService.h"
#include "VoiceStateWidget.h"
#include "UiServiceInterface.h"
#include "Input.h"
#include "UiProxyWidget.h"
#include "VoiceControllerWidget.h"
#include "VoiceUsersInfoWidget.h"

#include "DebugOperatorNew.h"

namespace CommUI
{
    VoiceToolWidget::VoiceToolWidget(Foundation::Framework* framework) : 
        framework_(framework),
        voice_users_info_widget_(0),
        in_world_voice_session_(0),
        voice_state_widget_(0),
        voice_controller_widget_(0),
        voice_controller_proxy_widget_(0),
        channel_selection_(0)
    {
        setupUi(this);
        InitializeInWorldVoice();
    }

    VoiceToolWidget::~VoiceToolWidget()
    {
        UninitializeInWorldVoice();
    }

    void VoiceToolWidget::InitializeInWorldVoice()
    {
        if (framework_ &&  framework_->GetServiceManager())
        {
            Communications::ServiceInterface *communication_service = framework_->GetService<Communications::ServiceInterface>();
            if (communication_service)
                ConnectInWorldVoiceSession( communication_service->InWorldVoiceSession() );
        }
    }

    void VoiceToolWidget::ConnectInWorldVoiceSession(Communications::InWorldVoice::SessionInterface* session)
    {
        in_world_voice_session_ = session;
        if (!session)
            return;

        QObject::connect(in_world_voice_session_, SIGNAL(StartSendingAudio()), this, SLOT(UpdateInWorldVoiceIndicator()) );
        QObject::connect(in_world_voice_session_, SIGNAL(StopSendingAudio()), this, SLOT(UpdateInWorldVoiceIndicator()) );
        QObject::connect(in_world_voice_session_, SIGNAL(StateChanged(Communications::InWorldVoice::SessionInterface::State)), this, SLOT(UpdateInWorldVoiceIndicator()));
        QObject::connect(in_world_voice_session_, SIGNAL(ParticipantJoined(Communications::InWorldVoice::ParticipantInterface*)), this, SLOT(UpdateInWorldVoiceIndicator()) );
        QObject::connect(in_world_voice_session_, SIGNAL(ParticipantJoined(Communications::InWorldVoice::ParticipantInterface*)), this, SLOT(ConnectParticipantVoiceAvticitySignals(Communications::InWorldVoice::ParticipantInterface*)) );
        QObject::connect(in_world_voice_session_, SIGNAL(ParticipantLeft(Communications::InWorldVoice::ParticipantInterface*)), this, SLOT(UpdateInWorldVoiceIndicator()) );
        QObject::connect(in_world_voice_session_, SIGNAL(destroyed()), this, SLOT(UninitializeInWorldVoice()));
        QObject::connect(in_world_voice_session_, SIGNAL(SpeakerVoiceActivityChanged(double)), this, SLOT(UpdateInWorldVoiceIndicator()));
        QObject::connect(in_world_voice_session_, SIGNAL(ActiceChannelChanged(QString)), this, SLOT(UpdateUI()));
        QObject::connect(in_world_voice_session_, SIGNAL(ChannelListChanged(QStringList)), this, SLOT(UpdateUI()));
    }

    void VoiceToolWidget::Minimize()
    {

    }

    void VoiceToolWidget::Maximize()
    {

    }

    void VoiceToolWidget::UpdateInWorldVoiceIndicator()
    {
        if (!in_world_voice_session_)
            return;

        if (in_world_voice_session_->GetState() != Communications::InWorldVoice::SessionInterface::STATE_OPEN)
        {
            voice_users_info_widget_->hide();
            voice_state_widget_->hide();
            if (voice_controller_proxy_widget_)
                voice_controller_proxy_widget_->hide();
            return;
        }
        else
        {
            voice_users_info_widget_->show();
            voice_state_widget_->show();
            //if (voice_controller_proxy_widget_)
            //{
            //    voice_controller_proxy_widget_->show();
            //}
        }

        if (in_world_voice_session_->IsAudioSendingEnabled())
        {
            if (voice_state_widget_)
            {
                voice_state_widget_->setState(CommUI::VoiceStateWidget::STATE_ONLINE);
                voice_state_widget_->SetVoiceActivity(in_world_voice_session_->SpeakerVoiceActivity());
            }
        }
        else
        {
            if (voice_state_widget_)
                voice_state_widget_->setState(CommUI::VoiceStateWidget::STATE_OFFLINE);
        }

        if (voice_users_info_widget_)
        {
            double channel_voice_activity = 0;
            QList<Communications::InWorldVoice::ParticipantInterface*> list = in_world_voice_session_->Participants();
            foreach(Communications::InWorldVoice::ParticipantInterface* p, list)
            {
                if (p->IsSpeaking())
                {
                    channel_voice_activity = 1;
                    break;
                }
            }
            voice_users_info_widget_->SetVoiceActivity(channel_voice_activity);
            voice_users_info_widget_->SetUsersCount(in_world_voice_session_->Participants().count());
        }
    }

    void VoiceToolWidget::ConnectParticipantVoiceAvticitySignals(Communications::InWorldVoice::ParticipantInterface* p)
    {
        connect(p, SIGNAL(StartSpeaking()), this, SLOT(UpdateInWorldVoiceIndicator()));
        connect(p, SIGNAL(StopSpeaking()), this, SLOT(UpdateInWorldVoiceIndicator()));
    }

    void VoiceToolWidget::UninitializeInWorldVoice()
    {
        if (voice_controller_proxy_widget_)
            framework_->UiService()->RemoveWidgetFromScene(voice_controller_proxy_widget_);
        if (voice_controller_widget_)
            SAFE_DELETE(voice_controller_widget_);
        in_world_voice_session_ = 0;
    }

    void VoiceToolWidget::ToggleVoiceControlWidget()
    {
        if (!in_world_voice_session_ || !voice_controller_proxy_widget_)
            return;

        if (voice_controller_proxy_widget_->isVisible())
            voice_controller_proxy_widget_->AnimatedHide();
        else
        {
            voice_controller_proxy_widget_->show();
            /// @todo fixme HACK BEGIN
            voice_controller_proxy_widget_->moveBy(1,1);
            voice_controller_proxy_widget_->moveBy(-1,-1);
            /// HACK END
        }
    }

    void VoiceToolWidget::UpdateUI()
    {
        QString channel = in_world_voice_session_->GetActiveChannel();

        if (!voice_state_widget_)
        {
            voice_state_widget_ = new VoiceStateWidget(0);
            connect(voice_state_widget_, SIGNAL( clicked() ), SLOT(ToggleVoiceControlWidget() ) );
            this->layout()->addWidget(voice_state_widget_);
            voice_state_widget_->show();
        }
        
        if (!voice_users_info_widget_)
        {
            voice_users_info_widget_ = new CommUI::VoiceUsersInfoWidget(0);
            connect(voice_users_info_widget_, SIGNAL( clicked() ), SLOT(ToggleVoiceControlWidget() ) );
            this->layout()->addWidget(voice_users_info_widget_);
        }
        voice_users_info_widget_->show();
        voice_users_info_widget_->updateGeometry();

        if (!channel_selection_)
        {
            channel_selection_ = new QComboBox();
            this->layout()->addWidget(channel_selection_);
        }
        
        disconnect(channel_selection_, SIGNAL(currentIndexChanged(const QString&)), in_world_voice_session_, SLOT(SetActiveChannel(QString)));
        channel_selection_->clear();
        channel_selection_->addItems(in_world_voice_session_->GetChannels());
        channel_selection_->addItems( QStringList() << ""); // no active channel
        channel_selection_->setCurrentIndex(channel_selection_->findText(in_world_voice_session_->GetActiveChannel()));
        channel_selection_->updateGeometry();
        connect(channel_selection_, SIGNAL(currentIndexChanged(const QString&)), in_world_voice_session_, SLOT(SetActiveChannel(QString)));
        this->update();

        if (voice_controller_widget_)
        {
            SAFE_DELETE(voice_controller_widget_);
            voice_controller_proxy_widget_ = 0; // automatically deleted by framework
        }
        if (in_world_voice_session_->GetState() == Communications::InWorldVoice::SessionInterface::STATE_OPEN)
        {
            voice_controller_widget_ = new VoiceControllerWidget(in_world_voice_session_);
            voice_controller_proxy_widget_ = framework_->UiService()->AddWidgetToScene(voice_controller_widget_);
            voice_controller_proxy_widget_->setWindowTitle("In-world voice");
            voice_controller_proxy_widget_->hide();
            /// @todo fixme HACK BEGIN
            voice_controller_proxy_widget_->moveBy(1,1);
            voice_controller_proxy_widget_->moveBy(-1,-1);
            /// HACK END

            if (framework_)
            {
                /// @todo Make these configurable
                input_context_ = framework_->GetInput()->RegisterInputContext("CommunicationWidget", 90);
                connect(input_context_.get(), SIGNAL(MouseMiddlePressed(MouseEvent*)), voice_controller_widget_, SLOT(SetPushToTalkOn()));
                connect(input_context_.get(), SIGNAL(MouseMiddleReleased(MouseEvent*)),voice_controller_widget_, SLOT(SetPushToTalkOff()));
                connect(input_context_.get(), SIGNAL(MouseMiddlePressed(MouseEvent*)), voice_controller_widget_, SLOT(Toggle()));
            }
        }
        UpdateInWorldVoiceIndicator();
    }
} // CommUI
