//
//  SWAbstract.hpp
//  SwitchWitch (Mac)
//
//  This is an abstract class that represents anything that could appear on the square.
//  Child classes: Unit, Obstacle (TBD)
//
//  Created by Ashley Yu on 3/11/22.
//  Copyright © 2022 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef SWSquareOccupant_hpp
#define SWSquareOccupant_hpp

#include <stdio.h>

#include <cugl/cugl.h>
using namespace cugl;

class SquareOccupant
{
public:
//    enum ObjectType{
//        ObjUnit,
//        ObjObstacle
//    };
    SquareOccupant() {};

    virtual ~SquareOccupant() {};
    
    //TODO: PLACEHOLDER FOR MORE COMMON PROPERTIES OF SQUAREOCCUPANTS
    
    virtual shared_ptr<cugl::scene2::SpriteNode> getViewNode()=0;
    
    virtual void setViewNode(shared_ptr<cugl::scene2::SpriteNode> viewNode)=0;
    
//    virtual void setObjectType(ObjectType objType) {
//        _objType = objType;
//    }
//
//    virtual ObjectType getObjectType() {
//        return _objType;
//    }
    
private:
    shared_ptr<cugl::scene2::PolygonNode> _viewNode;
    
//    ObjectType _objType;
    
};

#endif /* SWSquareOccupant_hpp */

