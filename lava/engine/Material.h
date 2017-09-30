#ifndef __LAVA_MATERIAL__
#define __LAVA_MATERIAL__

namespace lava
{
  namespace engine
  {
    struct BasicMaterial
    {
        enum MapType
        {
            DiffuseMap;
            SpecularMap;
            EmissiveMap;
            NormalMap;
            Count;
        };
        glm::vec3 diffuseColor = glm::vec3( 0.0f );
        glm::vec3 specularColor = glm::vec3( 0.0f );
        glm::vec3 emissiveColor = glm::vec3( 0.0f );
        float shininess = 128.0f;

        // todo: map for MapType
    };
    class Material: public std::enable_shared_from_this< Material >
    {
    public:
    	using Ptr = std::shared_ptr< Material >;

    	struct MaterialInfo
    	{
    		glm::vec4 albedo;
    		float shininnes;
    	};

    	static Material::Ptr create( void )
        {
            Material* m = new Material( );
            return Ptr( m );
        }
    protected:
    	Material( void );

    	bool _descDirty = false;
    	uint32_t textureCount = 0;
    };
  }
}

#endif /* __LAVA_MATERIAL__ */