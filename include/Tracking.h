/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef TRACKING_H
#define TRACKING_H

#include<opencv2/core/core.hpp>
#include<opencv2/features2d/features2d.hpp>

#include "Viewer.h"
#include "FrameDrawer.h"
#include "Map.h"
#include "LocalMapping.h"
#include "SurfelMapping.h"
#include "Frame.h"
#include "ORBVocabulary.h"
#include "KeyFrameDatabase.h"
#include "ORBextractor.h"
#include "MapDrawer.h"
#include "System.h"


#include <mutex>

#include "LSDextractor.h"
#include "MapLine.h"
#include "LSDmatcher.h"
#include "PlaneMatcher.h"
#include "MapPlane.h"
#include "Optimizer.h"

namespace ORB_SLAM2 {

    class Viewer;

    class FrameDrawer;

    class Map;

    class LocalMapping;

    class SurfelMapping;

    class System;

    class Tracking {

    public:

        Tracking(System *pSys, ORBVocabulary *pVoc, FrameDrawer *pFrameDrawer, MapDrawer *pMapDrawer, Map *pMap,
                 KeyFrameDatabase *pKFDB, const string &strSettingPath);

        cv::Mat GrabImage(const cv::Mat &imRGB, const cv::Mat &imD, const double &timestamp);

        void SetLocalMapper(LocalMapping *pLocalMapper);

//        void SetSurfelMapper(SurfelMapping *pSurfelMapper);

        void SetViewer(Viewer *pViewer);

        // Use this function if you have deactivated local mapping and you only want to localize the camera.
        void InformOnlyTracking(const bool &flag);

    public:

        // Tracking states
        enum eTrackingState {
            SYSTEM_NOT_READY = -1,
            NO_IMAGES_YET = 0,
            NOT_INITIALIZED = 1,
            OK = 2,
            LOST = 3
        };

        eTrackingState mState;
        eTrackingState mLastProcessedState;

        // Current Frame
        Frame mCurrentFrame;
        cv::Mat mImRGB;
        cv::Mat mImGray;
        cv::Mat mImDepth;

        // Lists used to recover the full camera trajectory at the end of the execution.
        // Basically we store the reference keyframe for each frame and its relative transformation
        list <cv::Mat> mlRelativeFramePoses;
        list<KeyFrame *> mlpReferences;
        list<double> mlFrameTimes;
        list<bool> mlbLost;

        // True if local mapping is deactivated and we are performing only localization
        bool mbOnlyTracking;

        void Reset();

    protected:

        // Main tracking function.
        void Track();
        void Track_object();

        // Map initialization for stereo and RGB-D
        void StereoInitialization();

        bool DetectManhattan();

        void CheckReplacedInLastFrame();

        bool TrackReferenceKeyFrame();

        void UpdateLastFrame();

        bool TrackWithMotionModel();

        bool TranslationEstimation();

        bool TranslationWithMotionModel();

//        bool Relocalization();

        void UpdateLocalMap();

        void UpdateLocalPoints();

        void UpdateLocalLines();

        void UpdateLocalKeyFrames();

        bool TrackLocalMap();

        void SearchLocalPoints();

        void SearchLocalLines();

        void SearchLocalPlanes();


        bool NeedNewKeyFrame();

        void CreateNewKeyFrame();

        // In case of performing only localization, this flag is true when there are no matches to
        // points in the map. Still tracking will continue if there are enough matches with temporal points.
        // In that case we are doing visual odometry. The system will try to do relocalization to recover
        // "zero-drift" localization to the map.
        bool mbVO;

        // Other Thread Pointers
        LocalMapping *mpLocalMapper;
//        SurfelMapping *mpSurfelMapper;

        // ORB
        ORBextractor *mpORBextractor;

        // Matchers
        LSDmatcher *mpLineMatcher;
        PlaneMatcher *mpPlaneMatcher;

        // Optimizer
        Optimizer *mpOptimizer;

        // BoW
        ORBVocabulary *mpORBVocabulary;
        KeyFrameDatabase *mpKeyFrameDB;

        // Local Map
        KeyFrame *mpReferenceKF;
        std::vector<KeyFrame *> mvpLocalKeyFrames;
        std::vector<MapPoint *> mvpLocalMapPoints;
        std::vector<MapLine *> mvpLocalMapLines;

        // System
        System *mpSystem;

        //Drawers
        Viewer *mpViewer;
        FrameDrawer *mpFrameDrawer;
        MapDrawer *mpMapDrawer;

        // Map
        Map *mpMap;

        // Calibration matrix
        cv::Mat mK;
        cv::Mat mDistCoef;
        float mbf;

        cv::Mat mUndistX, mUndistY;
        // New KeyFrame rules (according to fps)
        int mMinFrames;
        int mMaxFrames;

        // Threshold close/far points
        // Points seen as close by the stereo/RGBD sensor are considered reliable
        // and inserted from just one frame. Far points requiere a match in two keyframes.
        float mThDepth;

        // For RGB-D inputs only. For some datasets (e.g. TUM) the depthmap values are scaled.
        float mDepthMapFactor;

        //Current matches in frame
        int mnMatchesInliers;

        //Last Frame, KeyFrame and Relocalisation Info
        Frame mLastFrame;
        unsigned int mnLastKeyFrameId;
        unsigned int mnLastRelocFrameId;

        //Motion Model
        cv::Mat mVelocity;

        //Color order (true RGB, false BGR, ignored if grayscale)
        bool mbRGB;

        list<MapPoint *> mlpTemporalPoints;
        list<MapLine *> mlpTemporalLines;

        float mfMFVerTh;
        float mfDisTh;

        bool fullManhattanFound;

        cv::Mat manhattanRcw;

    };

} //namespace ORB_SLAM

#endif // TRACKING_H
