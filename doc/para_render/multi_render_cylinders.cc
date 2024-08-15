// #include <GL/glew.h>
// #include <GLFW/glfw3.h>

#include <mpi.h>
#include <vtkActor.h>
#include <vtkAutoInit.h>
#include <vtkCompositeRenderManager.h>
#include <vtkCylinderSource.h>
#include <vtkMPIController.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingFreeType);

// 创建圆柱体并添加到渲染器
vtkSmartPointer<vtkActor> CreateCylinder(int id) {
    vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
    cylinder->SetRadius(1.0);
    cylinder->SetHeight(2.0);
    cylinder->SetResolution(4);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cylinder->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->SetPosition(id, 0, 0);

    return actor;
}

int main(int argc, char* argv[]) {
    // if (!glfwInit()) {
    //     std::cerr << "Failed to initialize GLFW" << std::endl;
    //     return -1;
    // }

    // 初始化 MPI
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("multi render from rank %d out of %d processors\n", rank, size);
    // 初始化 VTK 的 MPI 控制器
    vtkSmartPointer<vtkMPIController> controller = vtkSmartPointer<vtkMPIController>::New();
    controller->Initialize(&argc, &argv, 1);

    if (controller->GetLocalProcessId() == 1) {
        std::cout << "Running in parallel mode." << std::endl;
    }
    vtkMultiProcessController::SetGlobalController(controller);

    // 创建渲染器和渲染窗口
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);

    controller->Barrier();
    // 创建渲染器管理器
    vtkSmartPointer<vtkCompositeRenderManager> renderManager = vtkSmartPointer<vtkCompositeRenderManager>::New();
    renderManager->SetRenderWindow(renderWindow);
    renderManager->SetController(controller);

    // 分配圆柱体给各个节点
    int cylindersPerNode = 5 / size;
    int start = rank * cylindersPerNode;
    int end = (rank == size - 1) ? 5 : (rank + 1) * cylindersPerNode;

    for (int i = start; i < end; ++i) {
        vtkSmartPointer<vtkActor> actor = CreateCylinder(i);
        renderer->AddActor(actor);
    }

    // 设置交互器
    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renderWindow);

    // 渲染和启动交互器
    if (rank == 0) {
        renderWindow->SetSize(800, 600);
        renderWindow->Render();
        renderManager->ResetAllCameras();
        iren->Start();
    } else {
        renderManager->StartServices();
    }
    // 终止 MPI
    // vtkMultiProcessController::SetGlobalController(nullptr);
    controller->Finalize();
    MPI_Finalize();

    return 0;
}