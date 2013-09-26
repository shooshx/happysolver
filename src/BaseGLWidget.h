#include "general.h"
#include "Mat.h"
#include "MatStack.h"
#include <list>

class BaseGLWidget;

class GLHandler {
public:
    GLHandler(BaseGLWidget* gl) : m_bgl(gl) {}

    /// this is a call back to whoever subclasses this widget and calls doChoise
    /// the default implementation does nothing.
    /// \arg \c inChoise a helper argument for user specified functionality. 
    ///	GLWidget always calls this method with inChoise == true.
    ///	if the user calls this function on his own he may want to do so with inChoise == false
    /// and do slightly differnt things, such us use more color and textures.
    virtual void drawTargets(bool inChoise) { } 
    virtual void myPaintGL() = 0;
    virtual void initialized() {} // called after the initialization is done for further gl-related init
    virtual void switchIn() {} // called when this handler becomes visible
    virtual void switchOut() {}

    // return true for default handling
    virtual void scrPress(bool rightButton, int x, int y) {  }
    virtual void scrRelease(bool rightButton) {  }
    virtual void scrMove(bool rightButton, bool ctrlPressed, int x, int y) {  }
    virtual bool scrDblClick(int x, int y) { return false; }
protected:
    BaseGLWidget *m_bgl;
};

class BaseGLWidget
{
public:
    BaseGLWidget();
    virtual ~BaseGLWidget() {}

    enum EAxis { Xaxis, Yaxis, Zaxis, XYaxis, XZaxis, YZaxis };

    virtual void checkErrors(const char* place);
    Mat4 transformMat();

    void switchHandler(GLHandler* handler);
    int doChoise(int chX, int chY);

public: 
    // this needs to be public for the PicPainter to be able to access it.
    list<int> m_textures;
    list<GLHandler*> m_handlers;
    MatStack proj, model;

    int m_minScaleReset;  //a view can set this member for to limit the maximal size of an object at reset. default is 0 for no effect
    bool m_cullFace;
    Vec3 aqmin, aqmax; // bbox for everything, floats as an optinimazation, no conversion 

protected:

    void reCalcProj(bool fFromScratch = true);

    void init();    
    void paint();
    void resize(int width, int height);
    /// perform actual selection of target.
    void callDrawTargets();
    void reset();
    void setNewMinMax(const Vec3& min, const Vec3& max, bool scale); 

    void rotate(EAxis axis, int x, int y);
    void zoom(int v); // v between 0 and 100
    void translate(int xValue, int yValue);
    void scale(int xValue, int yValue);

protected:
 
    EAxis m_axis;
    int m_zoomVal; // v between 0 and 100

    bool m_fUseLight;
    //Vec3 m_lightPos, m_lightColor;
    //float m_lightAmbient, m_lightDiffuse, m_lightSpecular;
    bool m_bSkewReset; // should the reset be slighty rotated for better view or not
    GLHandler *m_handler;

private:
    int m_cxClient;			///< hold the windows width
    int m_cyClient;			///< hold the windows height
    double m_aspectRatio;		///< hold the fixed Aspect Ration
    double m_scrScale, m_realScale;

};
