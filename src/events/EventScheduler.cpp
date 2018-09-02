#include "events/EventScheduler.h"
#include <algorithm>

EventScheduler* EventScheduler::instance = NULL;

namespace std {
    // Fix odd linker errors - please fix
    void __throw_bad_alloc() {while (1);}
    void __throw_length_error(char const* ignore) {while (1);}
}

EventScheduler::EventScheduler() {
}

void EventScheduler::update() {
  //printf("Event scheduler update \n");
  for (EventListener* listener : eventListeners) {
    listener->checkConditions();
  }

  for (Subsystem* subsystem : subsystems) {
    subsystem->initDefaultCommand();
    this->numSubsystems++;
  }
  subsystems.clear();

  std::vector<Subsystem*> usedSubsystems;

  Command* command; // Holds the command we're currently checking for run-ability
  bool canRun;

  printf("CommandQueue size is %u\n", commandQueue.size());
  for (int i = commandQueue.size() - 1; i >= 0; i--) {
    command = commandQueue[i];

    canRun = command->canRun();

    // First, check requirements, and if we can't run because of them,
    // then pop us off the commandQueue and pretend we don't exist
    std::vector<Subsystem*>& commandRequirements = command->getRequirements();

    if (usedSubsystems.size() == this->numSubsystems || !canRun) {
      // Shortcut to not iterate through the usedSubsystems vector each time
      canRun = false;
    } else {
      for (Subsystem* aSubsystem : commandRequirements) {
        if (std::find(usedSubsystems.begin(), usedSubsystems.end(), aSubsystem) != usedSubsystems.end()) {
          // If the subsystem that we want to use is already in usedSubsystems
          // (Quick sidenote: C++'s way of checking for object existence in
          // an array seems really stupid, but it's surprisingly useful!)
          // then we need to pop it off the queue, since we can't take control
          //
          // Remember: We're already going in order of priority, so we can't
          // take control anyways
          canRun = false;
          break;
        }
      }
    }
    if (canRun) {
      // Keep track of the subsystems we've already used
      usedSubsystems.insert(usedSubsystems.end(), commandRequirements.begin(), commandRequirements.end());

      if (!command->initialized) {
        command->initialize();
        command->initialized = true;
      }

      command->execute();

      if (command->isFinished()) {
        command->end();
        command->initialized = false;
        if (command->priority > 0) {
          // Not a default command, we can pop it off the commandQueue
          commandQueue.erase(commandQueue.begin() + i);
        }
      }
    } else {
      if (command->initialized) {
        command->interrupted();
        command->initialized = false;
      }
      if (command->priority > 0) {
        // We're not a default command (defined by having a priority of 0),
        // so there's no danger in discarding us
        commandQueue.erase(commandQueue.begin() + i);
      }
      //continue;
    }

    // We've proven that we can run, and since we're going in order of descending
    // priority, we don't need to worry abuot other commands using our requirements.
    // Therefore, we can set up, execute, and finish the command like normal
  }

  delay(5);
}

void EventScheduler::addCommand(Command* command) {
  if (!commandInQueue(command)) {
    if (commandQueue.size() == 0) {
      commandQueue.push_back(command);
      return;
    }

    // 0, 5, 10
    // trying to insert 7
    //

    for (size_t i = 0; i < commandQueue.size(); i++) {
      if (command->priority <= commandQueue[i]->priority) {
        commandQueue.insert(commandQueue.begin() + i, command);
        return;
      }
    }
    commandQueue.push_back(command);
  }
}

void EventScheduler::addEventListener(EventListener* eventListener) {
  this->eventListeners.push_back(eventListener);
}

void EventScheduler::trackSubsystem(Subsystem *aSubsystem) {
  this->subsystems.push_back(aSubsystem);
}

bool EventScheduler::commandInQueue(Command* aCommand) {
  return std::find(commandQueue.begin(), commandQueue.end(), aCommand) != commandQueue.end();
}

EventScheduler* EventScheduler::getInstance() {
    if (instance == NULL) {
        instance = new EventScheduler();
    }
    return instance;
}
