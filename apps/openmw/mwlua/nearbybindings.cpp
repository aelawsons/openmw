#include "luabindings.hpp"

#include <components/lua/luastate.hpp>
#include <components/queries/luabindings.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwphysics/raycasting.hpp"

#include "worldview.hpp"

namespace sol
{
    template <>
    struct is_automagical<MWPhysics::RayCastingResult> : std::false_type {};
}

namespace MWLua
{
    sol::table initNearbyPackage(const Context& context)
    {
        sol::table api(context.mLua->sol(), sol::create);
        WorldView* worldView = context.mWorldView;

        sol::usertype<MWPhysics::RayCastingResult> rayResult =
            context.mLua->sol().new_usertype<MWPhysics::RayCastingResult>("RayCastingResult");
        rayResult["hit"] = sol::readonly_property([](const MWPhysics::RayCastingResult& r) { return r.mHit; });
        rayResult["hitPos"] = sol::readonly_property([](const MWPhysics::RayCastingResult& r) -> sol::optional<osg::Vec3f>
        {
            if (r.mHit)
                return r.mHitPos;
            else
                return sol::nullopt;
        });
        rayResult["hitNormal"] = sol::readonly_property([](const MWPhysics::RayCastingResult& r) -> sol::optional<osg::Vec3f>
        {
            if (r.mHit)
                return r.mHitNormal;
            else
                return sol::nullopt;
        });
        rayResult["hitObject"] = sol::readonly_property([worldView](const MWPhysics::RayCastingResult& r) -> sol::optional<LObject>
        {
            if (r.mHitObject.isEmpty())
                return sol::nullopt;
            else
                return LObject(getId(r.mHitObject), worldView->getObjectRegistry());
        });

        constexpr int defaultCollisionType = MWPhysics::CollisionType_World | MWPhysics::CollisionType_HeightMap |
                                             MWPhysics::CollisionType_Actor | MWPhysics::CollisionType_Door;
        api["COLLISION_TYPE"] = LuaUtil::makeReadOnly(context.mLua->sol().create_table_with(
            "World", MWPhysics::CollisionType_World,
            "Door", MWPhysics::CollisionType_Door,
            "Actor", MWPhysics::CollisionType_Actor,
            "HeightMap", MWPhysics::CollisionType_HeightMap,
            "Projectile", MWPhysics::CollisionType_Projectile,
            "Water", MWPhysics::CollisionType_Water,
            "Default", defaultCollisionType));

        api["castRay"] = [defaultCollisionType](const osg::Vec3f& from, const osg::Vec3f& to, sol::optional<sol::table> options)
        {
            MWWorld::Ptr ignore;
            int collisionType = defaultCollisionType;
            float radius = 0;
            if (options)
            {
                sol::optional<LObject> ignoreObj = options->get<sol::optional<LObject>>("ignore");
                if (ignoreObj) ignore = ignoreObj->ptr();
                collisionType = options->get<sol::optional<int>>("collisionType").value_or(collisionType);
                radius = options->get<sol::optional<float>>("radius").value_or(0);
            }
            const MWPhysics::RayCastingInterface* rayCasting = MWBase::Environment::get().getWorld()->getRayCasting();
            if (radius <= 0)
                return rayCasting->castRay(from, to, ignore, std::vector<MWWorld::Ptr>(), collisionType);
            else
            {
                if (!ignore.isEmpty()) throw std::logic_error("Currently castRay doesn't support `ignore` when radius > 0");
                return rayCasting->castSphere(from, to, radius, collisionType);
            }
        };

        api["activators"] = LObjectList{worldView->getActivatorsInScene()};
        api["actors"] = LObjectList{worldView->getActorsInScene()};
        api["containers"] = LObjectList{worldView->getContainersInScene()};
        api["doors"] = LObjectList{worldView->getDoorsInScene()};
        api["items"] = LObjectList{worldView->getItemsInScene()};
        api["selectObjects"] = [context](const Queries::Query& query)
        {
            ObjectIdList list;
            WorldView* worldView = context.mWorldView;
            if (query.mQueryType == "activators")
                list = worldView->getActivatorsInScene();
            else if (query.mQueryType == "actors")
                list = worldView->getActorsInScene();
            else if (query.mQueryType == "containers")
                list = worldView->getContainersInScene();
            else if (query.mQueryType == "doors")
                list = worldView->getDoorsInScene();
            else if (query.mQueryType == "items")
                list = worldView->getItemsInScene();
            return LObjectList{selectObjectsFromList(query, list, context)};
            // TODO: Maybe use sqlite
            // return LObjectList{worldView->selectObjects(query, true)};
        };
        return LuaUtil::makeReadOnly(api);
    }
}
