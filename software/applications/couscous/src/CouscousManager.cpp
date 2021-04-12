
#include "CouscousManager.h"

#include "IHali.h"
#include "Posi.h"

CouscousManager::CouscousManager(
    const std::shared_ptr<IHali> &hali,
    const std::shared_ptr<Posi> &posi,
    const std::shared_ptr<WebServer> &web_server,
    const std::shared_ptr<Navi> &navi)
    : hali_(hali),
      posi_(posi),
      navi_(navi),
      web_server_(web_server) {
    web_server_thread_ = std::thread([&] { web_server_->start(); });
}

void CouscousManager::onPositionChanged(const PositionOrientation &position_orientation) {
    web_server_->setPosition(position_orientation);
    navi_->setCurrentPosition(
        position_orientation.x_m_,
        position_orientation.y_m_,
        position_orientation.orientation_rad_);
}

void CouscousManager::onWebServerPositionRequest(const PositionOrientation &position_orientation) {
    posi_->setPosition(position_orientation);
}

void CouscousManager::onWebServerSpeedRequest(const int &speed_motor1, const int &speed_motor2) {
    std::cout << "speed" << speed_motor1 << " " << speed_motor2 << std::endl;
    hali_->setMd25Speed(speed_motor1, speed_motor2);
}

void CouscousManager::onWebServerTargetPositionRequest(const double &pos_x, const double &pos_y) {
    // Will be sent to navi
    navi_->setTargetPosition(pos_x, pos_y, 0);
}

void CouscousManager::onNaviTargetReachedRequest(void) {}

// void CouscousManager::onNaviSpeedRequest(const int &speed_motor1, const int &speed_motor2) {
void CouscousManager::onNaviSpeedRequest(
    const float &v_x_mps,
    const float &v_y_mps,
    const float &omega_radps) {
    // std::cout << "couscous manager: navi set speed" << v_x_mps << std::endl;
    // To be done
    /*   std::cout << "set speed" << v_x_mps << " rotation " << omega_radps << std::endl;
       BaseSpeed bs(v_x_mps, v_y_mps, omega_radps);
       moti_.setBaseSpeed(bs);
       MotorSpeed ms = moti_.getMotorSpeed();
       hali_->setMd25Speed(ms.speed_motor_1_, ms.speed_motor_2_);
      */

    MotorSpeed motor_speed =
        moti_.getMotorSpeedFromBaseSpeed(BaseSpeed(v_x_mps, v_y_mps, omega_radps));
    hali_->setMd25Speed(motor_speed.speed_motor_1_, motor_speed.speed_motor_2_);
}

void CouscousManager::start() {
    hali_thread_ = std::thread([&] { hali_->updater(); });

    while (true) {
        auto encoders_motor1 = hali_->getEncoder(MotorIdEnum::motor1);
        auto encoders_motor2 = hali_->getEncoder(MotorIdEnum::motor2);

        posi_->updatePosition(encoders_motor1, encoders_motor2);
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_REFRESH_MS));
    }
    web_server_thread_.join();
    hali_thread_.join();
}