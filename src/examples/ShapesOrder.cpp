#include "Common.h"

/************************************************************************/
/* Drawing Commands                                                     */
/************************************************************************/

void tvgDrawCmds(tvg::Canvas* canvas)
{
    if (!canvas) return;

    // Create first scene
    auto scene1 = tvg::Scene::gen();
    auto scene1_raw = scene1.get();
    
    // Blue circle
    auto shape = tvg::Shape::gen();
    auto shape1_raw = shape.get();
    shape->appendCircle(300, 200, 100, 100); 
    shape->fill(0, 0, 255, 255);              
    scene1->push(move(shape));

    // Red circle
    shape = tvg::Shape::gen();
    auto shape2_raw = shape.get();
    shape->appendCircle(200, 200, 100, 100);   
    shape->fill(255, 0, 0, 255);            
    scene1->push(move(shape));

    // Green circle
    shape = tvg::Shape::gen();
    auto shape3_raw = shape.get();
    shape->appendCircle(250, 250, 100, 100);   
    shape->fill(0, 255, 0, 255);           
    scene1->push(move(shape));

    // Create second scene
    auto scene2 = tvg::Scene::gen();
    auto scene2_raw = scene2.get();

    // White rectangle
    shape = tvg::Shape::gen();
    shape->appendRect(200, 100, 300, 300, 0, 0); 
    shape->fill(255, 255, 255, 255);              
    scene2->push(move(shape));

    canvas->push(move(scene1));
    canvas->push(move(scene2));

    // Render order: ( white ) -> ( blue -> red -> green) 
    canvas->move_above(scene1_raw, scene2_raw);
    // Render order: ( white ) -> ( red -> green -> blue)
    scene1_raw->move_raise(shape1_raw);
    // Render order: ( white ) -> ( green -> red -> blue)
    scene1_raw->move_lower(shape3_raw);
    // Render order: ( white ) -> ( green -> blue -> red)
    scene1_raw->move_below(shape1_raw, shape2_raw);
}


/************************************************************************/
/* Sw Engine Test Code                                                  */
/************************************************************************/

static unique_ptr<tvg::SwCanvas> swCanvas;

void tvgSwTest(uint32_t* buffer)
{
    //Create a Canvas
    swCanvas = tvg::SwCanvas::gen();
    swCanvas->target(buffer, WIDTH, WIDTH, HEIGHT, tvg::SwCanvas::ARGB8888);

    /* Push the shape into the Canvas drawing list
       When this shape is into the canvas list, the shape could update & prepare
       internal data asynchronously for coming rendering.
       Canvas keeps this shape node unless user call canvas->clear() */
    tvgDrawCmds(swCanvas.get());
}

void drawSwView(void* data, Eo* obj)
{
    if (swCanvas->draw() == tvg::Result::Success) {
        swCanvas->sync();
    }
}


/************************************************************************/
/* GL Engine Test Code                                                  */
/************************************************************************/

static unique_ptr<tvg::GlCanvas> glCanvas;

void initGLview(Evas_Object *obj)
{
    static constexpr auto BPP = 4;

    //Create a Canvas
    glCanvas = tvg::GlCanvas::gen();
    glCanvas->target(nullptr, WIDTH * BPP, WIDTH, HEIGHT);

    /* Push the shape into the Canvas drawing list
       When this shape is into the canvas list, the shape could update & prepare
       internal data asynchronously for coming rendering.
       Canvas keeps this shape node unless user call canvas->clear() */
    tvgDrawCmds(glCanvas.get());
}

void drawGLview(Evas_Object *obj)
{
    auto gl = elm_glview_gl_api_get(obj);
    gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT);

    if (glCanvas->draw() == tvg::Result::Success) {
        glCanvas->sync();
    }
}


/************************************************************************/
/* Main Code                                                            */
/************************************************************************/

int main(int argc, char **argv)
{
    tvg::CanvasEngine tvgEngine = tvg::CanvasEngine::Sw;

    if (argc > 1) {
        if (!strcmp(argv[1], "gl")) tvgEngine = tvg::CanvasEngine::Gl;
    }

    //Initialize ThorVG Engine
    if (tvgEngine == tvg::CanvasEngine::Sw) {
        cout << "tvg engine: software" << endl;
    } else {
        cout << "tvg engine: opengl" << endl;
    }

    //Threads Count
    auto threads = std::thread::hardware_concurrency();

    //Initialize ThorVG Engine
    if (tvg::Initializer::init(tvgEngine, threads) == tvg::Result::Success) {

        elm_init(argc, argv);

        if (tvgEngine == tvg::CanvasEngine::Sw) {
            createSwView();
        } else {
            createGlView();
        }

        elm_run();
        elm_shutdown();

        //Terminate ThorVG Engine
        tvg::Initializer::term(tvgEngine);

    } else {
        cout << "engine is not supported" << endl;
    }
    return 0;
}
