/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
* Copyright 2015 Pelican Mapping
* http://osgearth.org
*
* osgEarth is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include <osgEarthAnnotation/LocalGeometryNode>
#include <osgEarthAnnotation/AnnotationRegistry>
#include <osgEarthAnnotation/AnnotationUtils>
#include <osgEarthFeatures/GeometryCompiler>
#include <osgEarthFeatures/GeometryUtils>
#include <osgEarth/Utils>

#define LC "[GeometryNode] "

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Features;


LocalGeometryNode::LocalGeometryNode(MapNode* mapNode) :
GeoPositionNode()
{
    LocalGeometryNode::setMapNode( mapNode );
    init( 0L );
}

LocalGeometryNode::LocalGeometryNode(MapNode*     mapNode,
                                     Geometry*    geom,
                                     const Style& style) :
GeoPositionNode(),
_geom    ( geom ),
_style   ( style )
{
    LocalGeometryNode::setMapNode( mapNode );
    init( 0L );
}


LocalGeometryNode::LocalGeometryNode(MapNode*     mapNode,
                                     osg::Node*   node,
                                     const Style& style) :
GeoPositionNode(),
_style   ( style )
{
    LocalGeometryNode::setMapNode( mapNode );
    init( 0L );
}

void
LocalGeometryNode::setMapNode(MapNode* mapNode)
{
    if ( mapNode != getMapNode() )
    {
        GeoPositionNode::setMapNode( mapNode );
        init(0L);
    }
}

void
LocalGeometryNode::initNode()
{
    osgEarth::clearChildren( getPositionAttitudeTransform() );

    if ( _node.valid() )
    {
        _node = AnnotationUtils::installOverlayParent( _node.get(), _style );

        getPositionAttitudeTransform()->addChild( _node.get() );

        applyRenderSymbology( getStyle() );

        setLightingIfNotSet( getStyle().has<ExtrusionSymbol>() );
    }
}


void
LocalGeometryNode::initGeometry(const osgDB::Options* dbOptions)
{
    osgEarth::clearChildren( getPositionAttitudeTransform() );

    if ( _geom.valid() )
    {
        osg::ref_ptr<Session> session;
        if ( getMapNode() )
            session = new Session(getMapNode()->getMap(), 0L, 0L, dbOptions);
        
        GeometryCompiler gc;
        osg::ref_ptr<osg::Node> node = gc.compile( _geom.get(), getStyle(), FilterContext(session) );
        if ( node.valid() )
        {
            node = AnnotationUtils::installOverlayParent( node.get(), getStyle() );

            getPositionAttitudeTransform()->addChild( node.get() );

            applyRenderSymbology( getStyle() );
        }
    }
}


void 
LocalGeometryNode::init(const osgDB::Options* options)
{    
    if ( _node.valid() )
    {
        initNode();
    }
    else
    {
        initGeometry( options );
    }
}


void
LocalGeometryNode::setStyle( const Style& style )
{
    _style = style;
    init( 0L );
}


void
LocalGeometryNode::setNode( osg::Node* node )
{
    _node = node;
    _geom = 0L;
    initNode();
}


void
LocalGeometryNode::setGeometry( Geometry* geom )
{
    _geom = geom;
    _node = 0L;
    initGeometry(0L);
}

//-------------------------------------------------------------------

OSGEARTH_REGISTER_ANNOTATION( local_geometry, osgEarth::Annotation::LocalGeometryNode );


LocalGeometryNode::LocalGeometryNode(MapNode*              mapNode,
                                     const Config&         conf,
                                     const osgDB::Options* dbOptions) :
GeoPositionNode( mapNode, conf )
{
    if ( conf.hasChild("geometry") )
    {
        Config geomconf = conf.child("geometry");
        _geom = GeometryUtils::geometryFromWKT( geomconf.value() );
    }

    conf.getObjIfSet( "style", _style );
    init( dbOptions );
}

Config
LocalGeometryNode::getConfig() const
{
    Config conf = GeoPositionNode::getConfig();
    conf.key() = "local_geometry";

    if ( !_style.empty() )
        conf.addObj( "style", _style );

    if ( _geom.valid() )
    {
        conf.add( Config("geometry", GeometryUtils::geometryToWKT(_geom.get())) );
    }

    return conf;
}
