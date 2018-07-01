#include "RobotBase.h"
#include "events/EventScheduler.h"

RobotBase::RobotBase() {
  robotInit();
}

void RobotBase::robotInit() {
  printf("Robot created.\n");
}

void RobotBase::autonInit() {
  printf("Default autonInit() function\n");
}

void RobotBase::autonPeriodic() {
  printf("Default autonPeriodic() function\n");
  EventScheduler::getInstance()->update();
}

void RobotBase::teleopInit() {
  printf("Default teleopInit() function\n");
}

void RobotBase::teleopPeriodic() {
    printf("Default teleopPeriodic() function\n");
    EventScheduler::getInstance()->update();
}

void RobotBase::disabledInit() {
  printf("Default disabledInit() function\n");
}

void RobotBase::disabledPeriodic() {
  printf("Default disabledPeriodic() function");
}

void RobotBase::doOneCycle() {
  // This is my "tribute" of sorts to Squeak Smalltalk, the most
  // influential programming language to me and the first language
  // I've ever learned.
  // `doOneCycle` is the name of the method in PasteUpMorph that
  // ran the `step` methods for all of its submorphs and essentially
  // made the world tick. Similarly, this method is the method that
  // makes the robot tick.
  if (isEnabled()) {
    if (isAutonomous()) {
      // Robot is in autonomous mode
      if (lastState == RobotState::Auton) {
        autonPeriodic();
      } else {
        lastState = RobotState::Auton;
        autonInit();
      }
    } else {
      // Robot is in teleop
      if (lastState == RobotState::Teleop) {
        teleopPeriodic();
      } else {
        lastState = RobotState::Teleop;
        teleopInit();
      }
    }
  } else {
    // Robot is currently disabled
    if (lastState == RobotState::Disabled) {
      disabledPeriodic();
    } else {
      lastState = RobotState::Disabled;
      disabledInit();
    }
  }
}

template<class RobotMain>
void _privateRunRobot() {
  RobotMain* robotInstance = RobotMain::getInstance();
  while (true) {
    RobotMain::doOneCycle();
  }
}

template<class RobotMain>
void runRobot() {
  taskCreate(_privateRunRobot<RobotMain>, TASK_DEFAULT_STACK_SIZE, NULL, TASK_PRIORITY_DEFAULT);
}
