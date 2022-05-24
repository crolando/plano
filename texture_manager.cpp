#include <texture_manager.h>

// implement members
std::unordered_map<GLuint, std::unique_ptr<QOpenGLTexture>> texture_manager::texture_owner;

// imgui-node-editor needs us to implement non-member Application_*Texture*() functions,
// and since we're in QT, we will use a subclass of QT's QOpenGLExtraFunctions class.
// So, we need these call-forwarder functions
void* Application_LoadTexture(char const* path)
{
    return texture_manager::LoadTexture(path);
}

void Application_DestroyTexture(ImTextureID texture)
{
    return texture_manager::DestroyTexture(texture);
}

int Application_GetTextureWidth(void* id)
{
    return texture_manager::GetTextureWidth(id);
}

int Application_GetTextureHeight(void* id)
{
    return texture_manager::GetTextureHeight(id);
}

void* texture_manager::LoadTexture(const char* path)
{    
    // The Application_Loadtexture call should be prepended to adjust for this project's directory layout.
    const QString data_prefix("../imgui-node-editor/examples/blueprints-example/");
    const QString build_path = QString(path).prepend(data_prefix);

    QImage qtex(build_path);
    if(qtex.isNull()) {
        exit(-1);
    }

    // Create a raw pointer so we construct the thing
    // which creates a OpenGL ID and uploads it to the graphics card.
    QOpenGLTexture* t_ptr = new QOpenGLTexture(QImage(qtex).mirrored());
    t_ptr->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    t_ptr->setMagnificationFilter(QOpenGLTexture::Linear);

    // now that we have an ID, we can put the pointer into a unique_ptr which
    // can safely carry it to the destruction function DestroyTexture
    GLuint gid = t_ptr->textureId();

    // Some book-keeping to maintain clear ownership and link between ID and pix data
    // std::unordered_map<GLuint, std::unique_ptr<QOpenGLTexture>> texture_owner;
    texture_owner[gid] = std::unique_ptr<QOpenGLTexture>(t_ptr);

    return (ImTextureID)gid;
}

void texture_manager::DestroyTexture(ImTextureID texture)
{
    //resture our GLuint from our void*
    GLuint gid = (GLuint)texture;

    //delete the texture on the graphics card side.
    texture_owner[gid]->destroy();

    // destroy the QOpenGLTexture
    texture_owner.erase(gid);
}

int texture_manager::GetTextureWidth(ImTextureID texture)
{
    //restore our GLuint from our void*
    GLuint gid = (GLuint)texture;

    // use hash table to lookup QOpenGLTexture unique pointer
    return texture_owner[gid]->width();

}

int texture_manager::GetTextureHeight(ImTextureID texture)
{
    //restore our GLuint from our void*
    GLuint gid = (GLuint)texture;

    // use hash table to lookup QOpenGLTexture unique pointer
    return texture_owner[gid]->height();
}
