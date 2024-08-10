0417

- VTK并行用到哪些内容:MPI,vtkMPIController、vtkcompositedsvnchronizedRenderers、cmake2)
- VTK并行相关知识如何使用3)
- demo验证

### VTK并行用到哪些内容:MPI,vtkMPIController、vtkcompositedsvnchronizedRenderers、cmake2)

#### MPI

介绍MPI及如何基于vtk使用MPI(Part4):https://blog.csdn.net/FastCAE/article/details/130205274

- MPI（message passing interface）信息传输接口，是一种用于分布式计算多节点之间通信的标准。
- MPI典型的通信类型包括：点对点通信（阻塞通信、非阻塞通信）、集体通信（广播、收集等）。
- MPI是一种标准，其应用依赖于它的具体实现。如MPICH、OpenMPI、IntelMPI、MSMPI等，其中MSMPI就是Windows系统中的常用的并行库。

vtkMPIController:

Process communication using MPI.

[vtkMPIController](https://vtk.org/doc/nightly/html/classvtkMPIController.html) is a concrete class which implements the abstract multi-process control methods defined in [vtkMultiProcessController](https://vtk.org/doc/nightly/html/classvtkMultiProcessController.html) using MPI (Message Passing Interface) cf. Using MPI / Portable Parallel Programming with the Message-Passing Interface, Gropp et al, MIT Press. It also provide functionality specific to MPI and not present in [vtkMultiProcessController](https://vtk.org/doc/nightly/html/classvtkMultiProcessController.html). Before any MPI communication can occur [Initialize()](https://vtk.org/doc/nightly/html/classvtkMPIController.html#aed3044fc12577432bc8373dac6ea665e) must be called by all processes. It is required to be called once, controllers created after this need not call [Initialize()](https://vtk.org/doc/nightly/html/classvtkMPIController.html#aed3044fc12577432bc8373dac6ea665e). At the end of the program [Finalize()](https://vtk.org/doc/nightly/html/classvtkMPIController.html#a5a6532a4b1720692550956b147585888) must be called by all processes.

The use of user-defined communicators are supported with the CreateSubController method. Note that a duplicate of the user defined communicator is used for internal communications (RMIs). This communicator has the same properties as the user one except that it has a new context which prevents the two communicators from interfering with each other.

使用MPI处理通信。

vtkMultiProcessController是一个具体的类，它使用MPI(消息传递接口)实现了vtkMultiProcessController中定义的抽象多进程控制方法，参见使用MPI /可移植并行编程与消息传递接口，Gropp等人，麻省理工学院出版社。它还提供特定于MPI的功能，而不是vtkMultiProcessController中提供的功能。在任何MPI通信发生之前，所有进程都必须调用Initialize()。它需要被调用一次，在此之后创建的控制器不需要调用Initialize()。在程序结束时，所有进程都必须调用Finalize()。

CreateSubController方法支持使用用户定义的通信器。注意，用户定义通信器的副本用于内部通信(rmi)。此通信器具有与用户通信器相同的属性，除了它具有一个新的上下文以防止两个通信器相互干扰。

##### 使用vtkMPIController的基本思路

- \1) 创建一个vtkMpiController对象；
- \2) void vtkMPIController::Initialize() 执行mpi初始化；
- \3) 调用通信接口进行通信；(send, receive等)
- \4) void vtkMPIController::Finalize()结束mpi通信。

### vtkSynchronizedRenderers

vtk官方类：https://vtk.org/doc/nightly/html/classvtkSynchronizedRenderers.html

用到的例子：https://gitlab.kitware.com/vtk/vtk/-/blob/v9.3.0/Rendering/Parallel/Testing/Cxx/TestClientServerRendering.cxx

#### 代码用例相关知识

官方介绍：使用vtkClientServerCompositePass测试客户机-服务器渲染。

系统所提供的功能.硬件系统是Server,操作系统是Client.

#### 代码思路

- MyProcess类
  - public:图像缩放因子(ImageReductionFactor),Execute, Controller(vtkMultiProcessController)
  - private:
    - void CreatePipeline(vtkRenderer* renderer)：创建可视化管道并把它添加到renderer
    - void SetupRenderPasses(vtkRenderer* renderer)：设置渲染通道

### vtkSMPTools

技术文档：https://blog.csdn.net/qq_40041064/article/details/137219168

一种算法加速机制

在VTK中开发之初，就有并行计算。vtkMultiThreader类于1997年引入，主要用于支持映像管道中的线程共享内存数据处理。截至目前，VTK包含的并行算法主要包括：

1） **vtkSMPTools**，这是线程处理的抽象类，在后台可以使用不同的库，例如线程构建块（TBB）、OpenMP和X-Kappi。典型的目标应用是由主流多核、线程中央处理器（CPU）提供的粗粒度共享内存计算，例如Intel的i5和i7架构。

2） **VTK-m**，这是用于新兴处理器架构（GPU和协处理器）的科学可视化算法工具包。VTK-m专为细粒度并发而设计，并提供可用于构建各种扩展算法的抽象数据和执行模型。

3） **New Algorithms**，该组件需要重新设计和实现算法和数据结构，以最好地利用新的并行计算硬件和库。为此，有必要确定各种可视化和数据分析任务的基本功能。

###  vtkParallelBFS

- line41：
  - vtkController的第三个参数1：布尔值，用于指示MPI环境是否已经被外部程序初始化。如果`initializedExternally`为`1`（或`true`），则表示MPI环境已经在外部初始化，`vtkMPIController`不会再调用`MPI_Init`。
  - 如果你已经调用了`MPI_Init(&argc, &argv);`，则`controller->Initialize(&argc, &argv, 1);`是正确的。
  - 如果你没有显式调用`MPI_Init(&argc, &argv);`，你应该使用`controller->Initialize(&argc, &argv, 0);`。

### vtk中使用mpi的步骤

- 安装和配置VTK与MPI

  - 编译vtk时的命令

  - ```
    cmake -B build -DBUILD_SHARED_LIBS=ON -DVTK_VERSIONED_INSTALL=OFF -DVTK_INSTALL_SDK=ON -DVTK_GROUP_ENABLE_Qt=YES -DCMAKE_BUILD_TYPE=Release -G"MinGW Makefiles" -DCMAKE_CXX_FLAGS=-fcommon -DCMAKE_C_FLAGS=-fcommon -DCMAKE_INSTALL_PREFIX=/install/dir -DVTK_MODULE_ENABLE_VTK_ParallelMPI:STRING=WANT -DVTK_USE_MPI=ON
    ```

  - 

- 引入必要的头文件

  ```cpp
  #include <vtk_mpi.h> 
  
  #include <vtkMPIController.h>
  ```

- 初始化MPI环境

  - ```cpp
    int main(int argc, char** argv) {
        MPI_Init(&argc, &argv);
    }
    ```

- 创建和初始化`vtkMPIController`

  - ```cpp
    vtkSmartPointer<vtkMPIController> controller = vtkSmartPointer<vtkMPIController>::New();
    controller->Initialize(&argc, &argv, 1); 
    ```

- 获取进程信息

  - ```cpp
    int rank = controller->GetLocalProcessId();
    int numProcs = controller->GetNumberOfProcesses();
    ```

- 并行计算和数据处理

  - ```cpp
    vtkSmartPointer<vtkPBGLRandomGraphSource> source = vtkSmartPointer<vtkPBGLRandomGraphSource>::New();
    source->SetNumberOfVertices(100000);
    source->SetNumberOfEdges(10000);
    source->StartWithTreeOn();
    
    // 广度优先搜索
    vtkSmartPointer<vtkPBGLBreadthFirstSearch> bfs = vtkSmartPointer<vtkPBGLBreadthFirstSearch>::New();
    bfs->SetInputConnection(source->GetOutputPort());
    vtkSmartPointer<vtkPBGLCollectGraph> collect = vtkSmartPointer<vtkPBGLCollectGraph>::New();
    collect->SetInputConnection(bfs->GetOutputPort());
    collect->Update(rank, numProcs, 0);
    
    ```

  - 如果是并行渲染是否需要数据源像这里的PBGLPBGLRandomGraphSource一样支持多进程？

- 主进程执行特定任务

  - ```cpp
    if (rank == 0) {}
    ```

-  结束MPI环境

- CMAKE构建，编译和运行程序

  - 运行时：

  - ```cpp
    mpirun -np <number_of_processes> ./TestParallism
    ```

### 各种MPI调研



msmpi不可跨平台使用

支持商用且支持Windows,Linux, MacOS系统的MPI库：MPICH, OpenMPI

#### 





## 相关知识

### 线程和进程

https://blog.csdn.net/weixin_51182368/article/details/125799573

### socket编程

socket编程https://blog.csdn.net/fuhanghang/article/details/114528098

client-server:https://blog.csdn.net/sugesi/article/details/72885103

### port,ip等

https://blog.csdn.net/YIUECHEN/article/details/104258552