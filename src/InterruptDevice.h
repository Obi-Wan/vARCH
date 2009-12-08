/* 
 * File:   InterruptDevice.h
 * Author: ben
 *
 * Created on 7 dicembre 2009, 20.37
 */

#ifndef _INTERRUPTDEVICE_H
#define	_INTERRUPTDEVICE_H

#include <queue>
#include <map>

using namespace std;

class InterruptHandler {
public:
  class InterruptsRecord : public pair<int,int> {
  public:
    InterruptsRecord(const int& _priority, const int& _device_id) :
        pair<int,int>(_priority, _device_id) { }

    int getPriority() const { return first; }
    int getDeviceId() const { return second; }

    bool operator<(const InterruptsRecord& o) const { return first < o.first; }
  };
  typedef priority_queue<InterruptsRecord> InterruptsQueue;

private:
  struct DeviceRecord {
    int priority;
    bool enabled;

    DeviceRecord(const int& _pr, const bool& _en) :
                        priority(_pr), enabled(_en) { }
  };
  typedef map<int,DeviceRecord> InterruptDevicesMap;

  InterruptDevicesMap interruptDevicesMap;
  InterruptsQueue interruptsQueue;

public:
  void interruptSignal(const int& device_id) {
    // Add the give interrupt to the queue of interruptions, just if it's enabled
    InterruptDevicesMap::iterator device = interruptDevicesMap.find(device_id);
    if ( (device != interruptDevicesMap.end()) && device->second.enabled ) {
      interruptsQueue.push( InterruptsRecord(device->second.priority, device_id) );
    }
  }

  void registerDevice(const int& device_id, const int& priority,
                      const bool& enabled) {
    if (interruptDevicesMap.find(device_id) != interruptDevicesMap.end()) {
      interruptDevicesMap.insert(
          InterruptDevicesMap::value_type(device_id,
                                          DeviceRecord(priority,enabled)));
    } else {
      // signal error
    }
  }

  bool hasInterruptReady() const { return !interruptsQueue.empty(); }
  const InterruptsRecord &getTopInterrupt() const { return interruptsQueue.top(); }
  void topInterruptServed() { interruptsQueue.pop(); }
};

class InterruptDevice {
public:
  InterruptDevice() : interruptId(-1), interruptPriority(-1), interruptHandler(NULL) { }
//  InterruptDevice(const InterruptDevice& orig);
//  virtual ~InterruptDevice();

  void initInterrupts(const int& priority, const int& id,
                      InterruptHandler *_interruptHandler) {
    interruptPriority = priority;
    interruptId = id;
    interruptHandler = _interruptHandler;
    interruptHandler->registerDevice(interruptId,interruptPriority,true);
  }

  virtual void checkInterruptEvents() { }
protected:

  int interruptPriority;
  int interruptId;

  InterruptHandler * interruptHandler;
};

#endif	/* _INTERRUPTDEVICE_H */

