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

class InterruptHandler {
public:
  class InterruptsRecord : public std::pair<int32_t, int32_t> {
  public:
    InterruptsRecord(const int& _priority, const int32_t & _device_id) :
        std::pair<int32_t, int32_t>(_priority, _device_id) { }

    int getPriority() const { return first; }
    int getDeviceId() const { return second; }

    bool operator<(const InterruptsRecord& o) const { return first < o.first; }
  };
  typedef std::priority_queue<InterruptsRecord> InterruptsQueue;

private:
  struct DeviceRecord {
    int32_t priority;
    bool enabled;

    DeviceRecord(const int32_t & _pr, const bool & _en) :
                        priority(_pr), enabled(_en) { }
  };
  typedef std::map<int32_t, DeviceRecord> InterruptDevicesMap;

  InterruptDevicesMap interruptDevicesMap;
  InterruptsQueue interruptsQueue;

public:
  void interruptSignal(const int& device_id)
  {
    // Add the give interrupt to the queue of interruptions, just if it's enabled
    InterruptDevicesMap::iterator device = interruptDevicesMap.find(device_id);
    if ( (device != interruptDevicesMap.end()) && device->second.enabled )
    {
      interruptsQueue.push( InterruptsRecord(device->second.priority, device_id) );
    }
  }

  void registerDevice(const int32_t & device_id, const int32_t & priority,
                      const bool & enabled)
  {
    if (interruptDevicesMap.find(device_id) != interruptDevicesMap.end())
    {
      interruptDevicesMap.insert(
          InterruptDevicesMap::value_type(device_id,
                                          DeviceRecord(priority,enabled)));
    }
    else
    {
      // signal error
    }
  }

  bool hasInterruptReady() const { return !interruptsQueue.empty(); }
  const InterruptsRecord & getTopInterrupt() const { return interruptsQueue.top(); }
  void topInterruptServed() { interruptsQueue.pop(); }
};

class InterruptDevice {
public:
  InterruptDevice() : interruptId(-1), interruptPriority(-1), interruptHandler(NULL) { }
//  InterruptDevice(const InterruptDevice& orig);
//  virtual ~InterruptDevice();

  void initInterrupts(const int32_t & priority, const int32_t & id,
                      InterruptHandler *_interruptHandler)
  {
    interruptPriority = priority;
    interruptId = id;
    interruptHandler = _interruptHandler;
    interruptHandler->registerDevice(interruptId, interruptPriority, true);
  }

  virtual void checkInterruptEvents() { }
  virtual ~InterruptDevice() { }
protected:

  int interruptId;
  int interruptPriority;

  InterruptHandler * interruptHandler;
};

#endif	/* _INTERRUPTDEVICE_H */

