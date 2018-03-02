/**
* Copyright (c) 2017, Lava
* All rights reserved.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**/

#include "Material.h"
#include <iostream>

#ifdef LAVA_USE_ASSIMP

namespace lava
{
  namespace utility
  {
    bool Material::operator ==( const Material &other ) const
    {
      if (
        ( _name == other._name ) &&
        ( _polygonMode == other._polygonMode ) &&
        ( _twoSided == other._twoSided ) &&
        ( _shadingMode == other._shadingMode ) &&
        ( _textures == other._textures ) &&
        ( _uniforms == other._uniforms ) &&
        ( _sources == other._sources ) &&
        ( _shininess == other._shininess ) &&
        ( _shininessStrength == other._shininessStrength ) &&
        ( _reflectivity == other._reflectivity ) &&
        ( _refractIndex == other._refractIndex ) &&
        ( _opacity == other._opacity ) &&
        ( _shininess == other._shininess )
        )
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    Material::Material( const aiMaterial* mtl, const std::string& globalPath )
    {
      unsigned int uiValue;
      float fValue;
      aiString aiStr;
      aiColor4D aiColor;
      std::string path;
      glm::vec4 color;

      //Name
      mtl->Get( AI_MATKEY_NAME, aiStr );
      setName( aiStr.C_Str( ) );

      //Polygon Mode
      uiValue = 1; //default 1
      int polygonMode = 0;
      if ( mtl->Get( AI_MATKEY_ENABLE_WIREFRAME, &polygonMode, &uiValue ) == AI_SUCCESS )
      {
        setPolygonMode( polygonMode ? PolygonMode::Line : PolygonMode::Fill );
      }
      else
      {
        setPolygonMode( PolygonMode::Fill );
      }

      //Two Sided
      uiValue = 1; //default 1
      int twoSided = 0;
      if ( ( mtl->Get( AI_MATKEY_TWOSIDED, &twoSided, &uiValue ) ) == AI_SUCCESS && twoSided )
      {
        setTwoSided( true );
      }
      else
      {
        setTwoSided( false );
      }

      //Shading mode
      aiShadingMode aiShadingMode = aiShadingMode::aiShadingMode_NoShading;
      if ( mtl->Get( AI_MATKEY_SHADING_MODEL, aiShadingMode ) == AI_SUCCESS )
      {
        setShadingMode( aiShadingMode );
      }

      //Ambient Texture
      path = getTexturePath( mtl, aiTextureType_AMBIENT );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Ambient, path );
      }

      //Diffuse Texture
      path = getTexturePath( mtl, aiTextureType_DIFFUSE );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Diffuse, path );
      }

      //Specular Texture
      path = getTexturePath( mtl, aiTextureType_SPECULAR );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Specular, path );
      }

      //Emissive Texture
      path = getTexturePath( mtl, aiTextureType_EMISSIVE );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Emissive, path );
      }

      //Opacity Texture
      path = getTexturePath( mtl, aiTextureType_OPACITY );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Opacity, path );
      }

      //Normals Texture
      path = getTexturePath( mtl, aiTextureType_NORMALS );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Normals, path );
      }

      //Lightmap Texture
      path = getTexturePath( mtl, aiTextureType_LIGHTMAP );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::LightMap, path );
      }

      //Height Texture
      path = getTexturePath( mtl, aiTextureType_HEIGHT );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Height, path );
      }

      //Shininess Texture
      path = getTexturePath( mtl, aiTextureType_SHININESS );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Shininess, path );
      }

      //Reflective Texture
      path = getTexturePath( mtl, aiTextureType_REFLECTION );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Reflective, path );
      }

      //Displacement Texture
      path = getTexturePath( mtl, aiTextureType_DISPLACEMENT );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Displacement, path );
      }

      //Unknown Texture
      path = getTexturePath( mtl, aiTextureType_UNKNOWN );
      path = globalPath + parsePath( path );
      if ( !path.empty( ) )
      {
        setTextureMaterial( MaterialType::Unknown, path );
      }

      //Ambient Uniform Color
      color = glm::vec4( 0.0f );
      color = getUniformColor( mtl, AI_MATKEY_COLOR_AMBIENT );
      if ( color != glm::vec4( 0.0f ) )
      {
        setUniformMaterial( MaterialType::Ambient, color );
      }

      //Diffuse Uniform Color
      color = glm::vec4( 0.0f );
      color = getUniformColor( mtl, AI_MATKEY_COLOR_DIFFUSE );
      if ( color != glm::vec4( 0.0f ) )
      {
        setUniformMaterial( MaterialType::Diffuse, color );
      }

      //Specular Uniform Color
      color = glm::vec4( 0.0f );
      color = getUniformColor( mtl, AI_MATKEY_COLOR_SPECULAR );
      if ( color != glm::vec4( 0.0f ) )
      {
        setUniformMaterial( MaterialType::Specular, color );
      }

      //Emissive Uniform Color
      color = glm::vec4( 0.0f );
      color = getUniformColor( mtl, AI_MATKEY_COLOR_EMISSIVE );
      if ( color != glm::vec4( 0.0f ) )
      {
        setUniformMaterial( MaterialType::Emissive, color );
      }

      //Transparent Uniform Color
      color = glm::vec4( 0.0f );
      color = getUniformColor( mtl, AI_MATKEY_COLOR_TRANSPARENT );
      if ( color != glm::vec4( 0.0f ) )
      {
        setUniformMaterial( MaterialType::Opacity, color ); //it's opacity or transparency?
      }

      //Transparent Uniform Color
      color = glm::vec4( 0.0f );
      color = getUniformColor( mtl, AI_MATKEY_COLOR_REFLECTIVE );
      if ( color != glm::vec4( 0.0f ) )
      {
        setUniformMaterial( MaterialType::Reflective, color );
      }

      //SCALAR PROPERTIES
      //Shininess
      uiValue = 1; //default 1
      fValue = 0.0f;
      if ( mtl->Get( AI_MATKEY_SHININESS, &fValue, &uiValue ) == AI_SUCCESS )
      {
        setShininess( fValue );
      }

      //Shininess Strength
      uiValue = 1; //default 1
      fValue = 1.0f; //default 1.0f
      if ( mtl->Get( AI_MATKEY_SHININESS_STRENGTH, &fValue, &uiValue ) == AI_SUCCESS )
      {
        setShininessStrength( fValue );
      }

      //Reflectivity
      fValue = 0.0f;
      if ( mtl->Get( AI_MATKEY_REFLECTIVITY, fValue ) == AI_SUCCESS )
      {
        setReflectivity( fValue );
      }

      //Refract Index
      fValue = 1.0f; //default 1.0f
      if ( mtl->Get( AI_MATKEY_REFRACTI, fValue ) == AI_SUCCESS )
      {
        setRefractIndex( fValue );
      }

      //Opacity
      fValue = 1.0f; //default 1.0f
      if ( mtl->Get( AI_MATKEY_OPACITY, fValue ) == AI_SUCCESS )
      {
        setOpacity( fValue );
      }

    }

    std::string Material::getName( void ) const
    {
      return _name;
    }

    void Material::setName( const std::string& name )
    {
      _name = name;
    }

    PolygonMode Material::getPolygonMode( void ) const
    {
      return _polygonMode;
    }

    void Material::setPolygonMode( const PolygonMode& polygonMode )
    {
      _polygonMode = polygonMode;
    }

    bool Material::getTwoSided( void ) const
    {
      return _twoSided;
    }

    void Material::setTwoSided( const bool& twoSided )
    {
      _twoSided = twoSided;
    }

    ShadingMode Material::getShadingMode( void ) const
    {
      return _shadingMode;
    }

    void Material::setShadingMode( const ShadingMode& shadingMode )
    {
      _shadingMode = shadingMode;
    }

    void Material::setShadingMode( const aiShadingMode& shadingMode )
    {
      switch ( shadingMode )
      {
        case aiShadingMode::aiShadingMode_Flat:
          _shadingMode = ShadingMode::Flat;
          break;
        case aiShadingMode::aiShadingMode_Gouraud:
          _shadingMode = ShadingMode::Gouraud;
          break;
        case aiShadingMode::aiShadingMode_Phong:
          _shadingMode = ShadingMode::Phong;
          break;
        case aiShadingMode::aiShadingMode_Blinn:
          _shadingMode = ShadingMode::Blinn;
          break;
      }
    }

    std::string Material::getTextureMaterial( const MaterialType& materialType ) const
    {
      if ( _textures.find( materialType ) != _textures.end( ) )
      {
        return _textures.at( materialType );
      }
      else
      {
        return std::string( );
      }
    }

    void Material::setTextureMaterial( const MaterialType& materialType, std::string texturePath )
    {
      _textures[ materialType ] = texturePath;

      if ( _sources.find( materialType ) != _sources.end( ) )
      {
        _sources[ materialType ] = ( _sources[ materialType ] == MaterialSource::UniformColor )
          ? MaterialSource::Texture_and_UniformColor
          : MaterialSource::Texture;
      }
      else
      {
        _sources[ materialType ] = MaterialSource::Texture;
      }
    }

    glm::vec4 Material::getUniformMaterial( const MaterialType& materialType ) const
    {
      if ( _uniforms.find( materialType ) != _uniforms.end( ) )
      {
        return _uniforms.at( materialType );
      }
      else
      {
        return glm::vec4( 0.0f );
      }
    }

    void Material::setUniformMaterial( const MaterialType& materialType, glm::vec4 uniformColor )
    {
      _uniforms[ materialType ] = uniformColor;

      if ( _sources.find( materialType ) != _sources.end( ) )
      {
        _sources[ materialType ] = ( _sources[ materialType ] == MaterialSource::Texture )
          ? MaterialSource::Texture_and_UniformColor
          : MaterialSource::UniformColor;
      }
      else
      {
        _sources[ materialType ] = MaterialSource::UniformColor;
      }
    }

    float Material::getShininess( void ) const
    {
      return _shininess;
    }

    void Material::setShininess( const float& shininess )
    {
      _shininess = shininess;
    }

    float Material::getShininessStrength( void ) const
    {
      return _shininessStrength;
    }

    void Material::setShininessStrength( const float& shininessStrength )
    {
      _shininessStrength = shininessStrength;
    }

    float Material::getReflectivity( void ) const
    {
      return _reflectivity;
    }

    void Material::setReflectivity( const float& reflectivity )
    {
      _reflectivity = reflectivity;
    }

    float Material::getRefractIndex( void ) const
    {
      return _refractIndex;
    }

    void Material::setRefractIndex( const float& refractIndex )
    {
      _refractIndex = refractIndex;
    }

    float Material::getOpacity( void ) const
    {
      return _opacity;
    }

    void Material::setOpacity( const float& opacity )
    {
      _opacity = opacity;
    }

    MaterialSource Material::getMaterialSource( const MaterialType& materialType ) const
    {
      if ( _sources.find( materialType ) != _sources.end( ) )
      {
        return _sources.at( materialType );
      }
      else
      {
        return MaterialSource::None;
      }
    }

    std::string Material::parsePath( std::string& path )
    {
      if ( path[ 0 ] == '.' )
      {
        path.erase( 0, 1 );
      }

      if ( path[ 0 ] == '/' || path[ 0 ] == '\\' )
      {
        path.erase( 0, 1 );
      }

      for ( auto &v : path )
      {
        if ( v == '\\' )
        {
          v = '/';
        }
      }
      return path;
    }

    std::string Material::getTexturePath( const aiMaterial* mtl, const aiTextureType& textureType )
    {
      std::string path;
      aiString aiStr;
      if ( mtl->GetTextureCount( textureType ) > 0 )
      {
        if ( mtl->GetTexture( textureType, 0, &aiStr, NULL, NULL, NULL, NULL, NULL ) == AI_SUCCESS )
        {
          path = aiStr.C_Str( );
        }
      }
      return path;
    }

    glm::vec4 Material::getUniformColor( const aiMaterial* mtl, const char* key, unsigned int type, unsigned int idx )
    {
      glm::vec4 color = glm::vec4( 0.0f, 0.0f, 0.0f, 0.0f );
      aiColor4D aiColor = aiColor4D( 0.0f, 0.0f, 0.0f, 0.0f );
      if ( mtl->Get( key, type, idx, aiColor ) == AI_SUCCESS )
      {
        color = glm::vec4( aiColor.r, aiColor.g, aiColor.b, aiColor.a );
      }
      return color;
    }

  }
}

#endif