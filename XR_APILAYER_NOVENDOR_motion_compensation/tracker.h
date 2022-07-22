// Copyright(c) 2022 Sebastian Veith

#pragma once
#include "pch.h"
#include "utility.h"

class OpenXrTracker
{
  public:
    OpenXrTracker();
    ~OpenXrTracker();
    bool Init();
    void beginSession(XrSession session);
    void endSession();
    bool ResetReferencePose(XrTime frameTime);
    bool GetPoseDelta(XrPosef& poseDelta, XrTime frameTime);
    void ModifyFilterStrength(bool trans, bool increase);

    bool m_IsCalibrated{false};
    bool m_ResetReferencePose{false};
    bool m_IsBindingSuggested{false};
    bool m_IsActionSetAttached{false};
    XrActionSet m_ActionSet{XR_NULL_HANDLE};
    XrAction m_TrackerPoseAction{XR_NULL_HANDLE};
    XrSpace m_TrackerSpace{XR_NULL_HANDLE};
    XrSpace m_ReferenceSpace{XR_NULL_HANDLE};

  private:
    bool GetPose(XrPosef& trackerPose, XrTime frameTime) const;

    XrSession m_Session{XR_NULL_HANDLE};
    XrPosef m_ReferencePose{xr::math::Pose::Identity()};
    XrPosef m_LastPoseDelta{xr::math::Pose::Identity()};
    XrTime m_LastPoseTime{0};
    float m_TransStrength, m_RotStrength;
    utility::FilterBase<XrVector3f>* m_TransFilter = nullptr;
    utility::FilterBase<XrQuaternionf>* m_RotFilter = nullptr;
};