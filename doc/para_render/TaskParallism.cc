// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#ifndef __TASKPARA_H
#define __TASKPARA_H

#include "vtkActor.h"
#include "vtkAssignAttribute.h"
#include "vtkAttributeDataToFieldDataFilter.h"
#include "vtkCamera.h"
#include "vtkContourFilter.h"
#include "vtkDataSetMapper.h"
#include "vtkFieldDataToAttributeDataFilter.h"
#include "vtkGlyph3D.h"
#include "vtkGlyphSource2D.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageGradient.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkImageShrink3D.h"
#include "vtkMPIController.h"
#include "vtkProbeFilter.h"
#include "vtkRTAnalyticSource.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"

typedef vtkPolyDataMapper* (*taskFunction)(vtkRenderWindow* renWin, double data, vtkCamera* cam);

vtkPolyDataMapper* task1(vtkRenderWindow* renWin, double data, vtkCamera* cam);
vtkPolyDataMapper* task2(vtkRenderWindow* renWin, double data, vtkCamera* cam);

static const double EXTENT = 20;

static const int WINDOW_WIDTH = 400;
static const int WINDOW_HEIGHT = 300;

#endif

#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"

// Task 1 for TaskParallelism.
// See TaskParallelism.cxx for more information.
vtkPolyDataMapper* task1(vtkRenderWindow* renWin, double data, vtkCamera* cam) {
    double extent = data;
    int iextent = static_cast<int>(data);
    // The pipeline

    // Synthetic image source.
    vtkRTAnalyticSource* source1 = vtkRTAnalyticSource::New();
    source1->SetWholeExtent(-1 * iextent, iextent, -1 * iextent, iextent, -1 * iextent, iextent);
    source1->SetCenter(0, 0, 0);
    source1->SetStandardDeviation(0.5);
    source1->SetMaximum(255.0);
    source1->SetXFreq(60);
    source1->SetXMag(10);
    source1->SetYFreq(30);
    source1->SetYMag(18);
    source1->SetZFreq(40);
    source1->SetZMag(5);
    source1->GetOutput()->SetSpacing(2.0 / extent, 2.0 / extent, 2.0 / extent);

    // Iso-surfacing.
    vtkContourFilter* contour = vtkContourFilter::New();
    contour->SetInputConnection(source1->GetOutputPort());
    contour->SetNumberOfContours(1);
    contour->SetValue(0, 220);

    // Magnitude of the gradient vector.
    vtkImageGradientMagnitude* magn = vtkImageGradientMagnitude::New();
    magn->SetDimensionality(3);
    magn->SetInputConnection(source1->GetOutputPort());

    // Probe magnitude with iso-surface.
    vtkProbeFilter* probe = vtkProbeFilter::New();
    probe->SetInputConnection(contour->GetOutputPort());
    probe->SetSourceConnection(magn->GetOutputPort());
    probe->SpatialMatchOn();

    // Rendering objects.
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInputData(probe->GetPolyDataOutput());
    mapper->SetScalarRange(50, 180);

    vtkActor* actor = vtkActor::New();
    actor->SetMapper(mapper);

    vtkRenderer* ren = vtkRenderer::New();
    renWin->AddRenderer(ren);

    ren->AddActor(actor);
    ren->SetActiveCamera(cam);

    // Cleanup
    source1->Delete();
    contour->Delete();
    magn->Delete();
    probe->Delete();
    actor->Delete();
    ren->Delete();

    return mapper;
}

// Task 2 for TaskParallelism.
// See TaskParallelism.cxx for more information.
vtkPolyDataMapper* task2(vtkRenderWindow* renWin, double data, vtkCamera* cam) {
    double extent = data;
    int iextent = static_cast<int>(data);
    // The pipeline

    // Synthetic image source.
    vtkRTAnalyticSource* source1 = vtkRTAnalyticSource::New();
    source1->SetWholeExtent(-1 * iextent, iextent, -1 * iextent, iextent, -1 * iextent, iextent);
    source1->SetCenter(0, 0, 0);
    source1->SetStandardDeviation(0.5);
    source1->SetMaximum(255.0);
    source1->SetXFreq(60);
    source1->SetXMag(10);
    source1->SetYFreq(30);
    source1->SetYMag(18);
    source1->SetZFreq(40);
    source1->SetZMag(5);
    source1->GetOutput()->SetSpacing(2.0 / extent, 2.0 / extent, 2.0 / extent);

    // Gradient vector.
    vtkImageGradient* grad = vtkImageGradient::New();
    grad->SetDimensionality(3);
    grad->SetInputConnection(source1->GetOutputPort());

    vtkImageShrink3D* mask = vtkImageShrink3D::New();
    mask->SetInputConnection(grad->GetOutputPort());
    mask->SetShrinkFactors(5, 5, 5);

    // Label the scalar field as the active vectors.
    vtkAssignAttribute* aa = vtkAssignAttribute::New();
    aa->SetInputConnection(mask->GetOutputPort());
    aa->Assign(
        vtkDataSetAttributes::SCALARS, vtkDataSetAttributes::VECTORS, vtkAssignAttribute::POINT_DATA);

    vtkGlyphSource2D* arrow = vtkGlyphSource2D::New();
    arrow->SetGlyphTypeToArrow();
    arrow->SetScale(0.2);
    arrow->FilledOff();

    // Glyph the gradient vector (with arrows)
    vtkGlyph3D* glyph = vtkGlyph3D::New();
    glyph->SetInputConnection(aa->GetOutputPort());
    glyph->SetSourceConnection(arrow->GetOutputPort());
    glyph->ScalingOff();
    glyph->OrientOn();
    glyph->SetVectorModeToUseVector();
    glyph->SetColorModeToColorByVector();

    // Rendering objects.
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(glyph->GetOutputPort());
    mapper->SetScalarRange(50, 180);

    vtkActor* actor = vtkActor::New();
    actor->SetMapper(mapper);

    vtkRenderer* ren = vtkRenderer::New();
    renWin->AddRenderer(ren);

    ren->AddActor(actor);
    ren->SetActiveCamera(cam);

    // Cleanup
    source1->Delete();
    grad->Delete();
    aa->Delete();
    mask->Delete();
    glyph->Delete();
    arrow->Delete();
    actor->Delete();
    ren->Delete();

    return mapper;
}

#include "vtkAutoInit.h"
#include "vtkCompositeRenderManager.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingFreeType);

// This function sets up properties common to both processes
// and executes the task corresponding to the current process
void process(vtkMultiProcessController* controller, void* vtkNotUsed(arg)) {
    taskFunction task;
    int myId = controller->GetLocalProcessId();

    // Chose the appropriate task (see task1.cxx and task2.cxx)
    if (myId == 0) {
        task = task1;
    } else {
        task = task2;
    }

    // Setup camera
    vtkCamera* cam = vtkCamera::New();
    cam->SetPosition(-0.6105, 1.467, -6.879);
    cam->SetFocalPoint(-0.0617558, 0.127043, 0);
    cam->SetViewUp(-0.02, 0.98, 0.193);
    cam->SetClippingRange(3.36, 11.67);
    cam->Dolly(0.8);

    // Create the render objects
    vtkRenderWindow* renWin = vtkRenderWindow::New();
    renWin->SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);

    // This class allows all processes to composite their images.
    // The root process then displays it in it's render window.
    vtkCompositeRenderManager* tc = vtkCompositeRenderManager::New();
    tc->SetRenderWindow(renWin);

    // Generate the pipeline see task1.cxx and task2.cxx)
    vtkPolyDataMapper* mapper = (*task)(renWin, EXTENT, cam);

    // Only the root process will have an active interactor. All
    // the other render windows will be slaved to the root.
    tc->StartInteractor();

    // Clean-up
    iren->Delete();
    if (mapper) {
        mapper->Delete();
    }
    renWin->Delete();
    cam->Delete();
}

int main(int argc, char* argv[]) {
    // Note that this will create a vtkMPIController if MPI
    // is configured, vtkThreadedController otherwise.
    vtkMPIController* controller = vtkMPIController::New();
    controller->Initialize(&argc, &argv);

    // When using MPI, the number of processes is determined
    // by the external program which launches this application.
    // However, when using threads, we need to set it ourselves.
    if (controller->IsA("vtkThreadedController")) {
        // Set the number of processes to 2 for this example.
        controller->SetNumberOfProcesses(2);
    }
    int numProcs = controller->GetNumberOfProcesses();

    if (numProcs != 2) {
        cerr << "This example requires two processes." << endl;
        controller->Finalize();
        controller->Delete();
        return 1;
    }

    // Execute the function named "process" on both processes
    controller->SetSingleMethod(process, 0);
    controller->SingleMethodExecute();

    // Clean-up and exit
    controller->Finalize();
    controller->Delete();

    return 0;
}
