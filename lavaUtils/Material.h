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

#ifndef __LAVAUTILS_MATERIAL__
#define __LAVAUTILS_MATERIAL__

#ifdef LAVA_USE_ASSIMP

#include <assimp/material.h>

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <functional>

#include <lavaUtils/api.h>

namespace lava
{
  namespace utility
  {

    enum class MaterialType : std::int8_t
    {
      Ambient = 1,
      Diffuse = 2,
      Specular = 3,
      Emissive = 4,
      Opacity = 5, //transparent uniform, opacity texture
      Normals = 6,
      LightMap = 7,
      Height = 8,
      Shininess = 9,
      Reflective = 10,
      Displacement = 11,
      Unknown = 12
    };

    enum class MaterialSource : std::int8_t
    {
      None = 0,
      Texture = 1,
      UniformColor = 2,
      Texture_and_UniformColor = 3
    };

    enum class ShadingMode
    {
      //NoShading,
      Flat,
      Gouraud,
      Phong,
      Blinn,
      //Toon,
      //OrenNayar,
      //Minnaert,
      //CookTorrance,
      //Fresnel
    };

    enum class PolygonMode
    {
      Point,
      Line,
      Fill
    };

    class Material
    {
      public:

      bool operator==( const Material &other ) const;

      LAVAUTILS_API
        Material( const aiMaterial* mtl, const std::string& globalPath );

      LAVAUTILS_API
        std::string getName( void ) const;

      LAVAUTILS_API
        void setName( const std::string& name );

      LAVAUTILS_API
        PolygonMode getPolygonMode( void ) const;

      LAVAUTILS_API
        void setPolygonMode( const PolygonMode& polygonMode );

      LAVAUTILS_API
        bool getTwoSided( void ) const;

      LAVAUTILS_API
        void setTwoSided( const bool& twoSided );

      LAVAUTILS_API
        ShadingMode getShadingMode( void ) const;

      LAVAUTILS_API
        void setShadingMode( const ShadingMode& shadingMode );

      LAVAUTILS_API
        void setShadingMode( const aiShadingMode& shadingMode );

      LAVAUTILS_API
        std::string getTextureMaterial( const MaterialType& materialType ) const;

      LAVAUTILS_API
        void setTextureMaterial( const MaterialType& materialType, std::string texturePath );

      LAVAUTILS_API
        glm::vec4 getUniformMaterial( const MaterialType& materialType ) const;

      LAVAUTILS_API
        void setUniformMaterial( const MaterialType& materialType, glm::vec4 uniformColor );

      LAVAUTILS_API
        float getShininess( void ) const;

      LAVAUTILS_API
        void setShininess( const float& shininess );

      LAVAUTILS_API
        float getShininessStrength( void ) const;

      LAVAUTILS_API
        void setShininessStrength( const float& shininessStrength );

      LAVAUTILS_API
        float getReflectivity( void ) const;

      LAVAUTILS_API
        void setReflectivity( const float& reflectivity );

      LAVAUTILS_API
        float getRefractIndex( void ) const;

      LAVAUTILS_API
        void setRefractIndex( const float& refractIndex );

      LAVAUTILS_API
        float getOpacity( void ) const;

      LAVAUTILS_API
        void setOpacity( const float& opacity );

      LAVAUTILS_API
        MaterialSource getMaterialSource( const MaterialType& materialType ) const;

      private:
      std::string _name;
      PolygonMode _polygonMode; //TODO: change for vk::PolygonMode _polygonMode;
      bool _twoSided; //TODO: change for...
      ShadingMode _shadingMode;
      std::unordered_map< MaterialType, std::string> _textures;
      std::unordered_map< MaterialType, glm::vec4 > _uniforms;
      std::unordered_map< MaterialType, MaterialSource > _sources;
      float _shininess;
      float _shininessStrength;
      float _reflectivity;
      float _refractIndex;
      float _opacity;

      std::string parsePath( std::string& path );
      std::string getTexturePath( const aiMaterial* mtl, const aiTextureType& textureType );
      glm::vec4 getUniformColor( const aiMaterial* mtl,
        const char* key, unsigned int type, unsigned int idx );
    };
  }
}

#endif

#endif /* __LAVAUTILS_MATERIAL__  */
