#ifndef OPENMW_MWRENDER_WATER_H
#define OPENMW_MWRENDER_WATER_H

#include <osg/ref_ptr>

#include "../mwworld/cellstore.hpp"

namespace osg
{
    class Group;
    class PositionAttitudeTransform;
}

namespace osgUtil
{
    class IncrementalCompileOperation;
}

namespace Resource
{
    class ResourceSystem;
}

namespace MWRender
{

    /// Water rendering
    class Water
    {
        static const int CELL_SIZE = 8192;

        osg::ref_ptr<osg::Group> mParent;
        osg::ref_ptr<osg::PositionAttitudeTransform> mWaterNode;
        Resource::ResourceSystem* mResourceSystem;
        osg::ref_ptr<osgUtil::IncrementalCompileOperation> mIncrementalCompileOperation;

        bool mEnabled;
        bool mToggled;
        float mTop;

        osg::Vec3f getSceneNodeCoordinates(int gridX, int gridY);
        void updateVisible();

    public:
        Water(osg::Group* parent, Resource::ResourceSystem* resourceSystem, osgUtil::IncrementalCompileOperation* ico);
        ~Water();

        void setEnabled(bool enabled);

        bool toggle();

        bool isUnderwater(const osg::Vec3f& pos) const;

        /*
        /// adds an emitter, position will be tracked automatically using its scene node
        void addEmitter (const MWWorld::Ptr& ptr, float scale = 1.f, float force = 1.f);
        void removeEmitter (const MWWorld::Ptr& ptr);
        void updateEmitterPtr (const MWWorld::Ptr& old, const MWWorld::Ptr& ptr);
        */

        void changeCell(const MWWorld::CellStore* store);
        void setHeight(const float height);

    };

}

#endif
