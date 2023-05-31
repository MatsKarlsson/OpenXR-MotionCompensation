// Copyright(c) 2022 Sebastian Veith

#pragma once
#include "utility.h"
#include "filter.h"

namespace Tracker
{
    class ControllerBase
    {
      public:
        virtual ~ControllerBase() = default;
        virtual bool Init();
        bool GetPoseDelta(XrPosef& poseDelta, XrSession session, XrTime time);
        virtual bool ResetReferencePose(XrSession session, XrTime time);

        bool m_ResetReferencePose{false};

      protected:
        virtual void SetReferencePose(const XrPosef& pose);
        virtual bool GetPose(XrPosef& trackerPose, XrSession session, XrTime time) = 0;
        virtual bool GetControllerPose(XrPosef& trackerPose, XrSession session, XrTime time);
        static XrVector3f GetForwardVector(const XrQuaternionf& quaternion, bool inverted = false);
        static XrQuaternionf GetYawRotation(const XrVector3f& forward);

        XrPosef m_ReferencePose{xr::math::Pose::Identity()};
        XrPosef m_LastPoseDelta{xr::math::Pose::Identity()};
        XrPosef m_LastPose{xr::math::Pose::Identity()};

    private:
        virtual void ApplyFilters(XrPosef& trackerPose){};

        bool m_PhysicalEnabled{false};
        bool m_ConnectionLost{false};
        XrTime m_LastPoseTime{0};
    };
   
    class TrackerBase : public ControllerBase
    {
      public:
        ~TrackerBase() override;
        bool Init() override;
        virtual bool LazyInit(XrTime time);
        void ModifyFilterStrength(bool trans, bool increase);
        void AdjustReferencePose(const XrPosef& pose);
        [[nodiscard]] XrPosef GetReferencePose() const;
        void SetReferencePose(const XrPosef& pose) override;
        void LogCurrentTrackerPoses(XrSession session, XrTime time, bool activated);
        virtual void ApplyCorManipulation(XrSession session, XrTime time){};

        bool m_SkipLazyInit{false};
        bool m_Calibrated{false};
       
      protected:
        void ApplyFilters(XrPosef& trackerPose) override;

      private:
        bool LoadFilters();

        float m_TransStrength{0.0f};
        float m_RotStrength{0.0f};
        Filter::FilterBase<XrVector3f>* m_TransFilter = nullptr;
        Filter::FilterBase<XrQuaternionf>* m_RotFilter = nullptr;
    };

    class OpenXrTracker final : public TrackerBase
    {
      public:
       bool ResetReferencePose(XrSession session, XrTime time) override;

      protected:
        bool GetPose(XrPosef& trackerPose, XrSession session, XrTime time) override;
    };

    class CorManipulator;

    class VirtualTracker : public TrackerBase
    {
      public:
        bool Init() override;
        bool LazyInit(XrTime time) override;
        bool ResetReferencePose(XrSession session, XrTime time) override;
        bool ChangeOffset(XrVector3f modification);
        bool ChangeRotation(bool right);
        void SaveReferencePose(XrTime time) const;
        void ApplyCorManipulation(XrSession session, XrTime time) override;

      protected:
        bool GetPose(XrPosef& trackerPose, XrSession session, XrTime time) override;
        virtual bool GetVirtualPose(XrPosef& trackerPose, XrSession session, XrTime time) = 0;

        std::string m_Filename;
        utility::Mmf m_Mmf;
        float m_OffsetForward{0.0f}, m_OffsetDown{0.0f}, m_OffsetRight{0.0f};
        bool m_UpsideDown{false};
        

      private:
        bool LoadReferencePose(XrSession session, XrTime time);

        std::unique_ptr<CorManipulator> m_Manipulator{};
        bool m_LoadPoseFromFile{false};
    };

    class YawTracker final : public VirtualTracker
    {
      public:
        YawTracker()
        {
            m_Filename = "Local\\YawVRGEFile";
        }
        bool ResetReferencePose(XrSession session, XrTime time) override;

      protected:
        bool GetVirtualPose(XrPosef& trackerPose, XrSession session, XrTime time) override;

      private:
        struct YawData
        {
            float yaw, pitch, roll, battery, rotationHeight, rotationForwardHead;
            bool sixDof, usePos;
            float autoX, autoY;
        };
    };

    class SixDofTracker : public VirtualTracker
    {
      protected:
        bool GetVirtualPose(XrPosef& trackerPose, XrSession session, XrTime time) override;
       
        bool m_IsSrs{false};

      private:
        struct SixDofData
        {
            double sway;
            double surge;
            double heave;
            double yaw;
            double roll;
            double pitch;
        };
    };

    class FlyPtTracker final : public SixDofTracker
    {
      public:
        FlyPtTracker()
        {
            m_Filename = "Local\\motionRigPose";
            m_IsSrs = false;
        }
    };

    class SrsTracker final : public SixDofTracker
    {
      public:
        SrsTracker()
        {
            m_Filename = "Local\\SimRacingStudioMotionRigPose";
            m_IsSrs = true;
        }
    };

    class CorManipulator : public ControllerBase
    {
      public:
        explicit CorManipulator(TrackerBase* tracker) : m_Tracker(tracker){};
        void ApplyManipulation(XrSession session, XrTime time);

      protected:
        bool GetPose(XrPosef& trackerPose, XrSession session, XrTime time) override;

      private:
        void GetButtonState(XrSession session, bool& moveButton, bool& positionButton);
        void ApplyPosition() const;
        void ApplyRotation() const;
        void ApplyTranslation() const;

        TrackerBase* m_Tracker{nullptr};
        bool m_Initialized{true};
        bool m_MoveActive{false};
        bool m_PositionActive{false};
    };
    
    struct ViveTrackerInfo
    {
        bool Init();

        bool active{false};
        std::string role;
        const std::string profile{"/interaction_profiles/htc/vive_tracker_htcx"};
    };

    constexpr float angleToRadian{static_cast<float>(M_PI) / 180.0f};

    void GetTracker(TrackerBase** tracker);
} // namespace Tracker