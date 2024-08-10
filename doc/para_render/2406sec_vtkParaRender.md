## Windows配置MSMPI并编译vtk

- GitHub：https://github.com/microsoft/Microsoft-MPI/releases下载msmpisetup.exe和msmpisdk.msi
  - vtk编译需要msmpisdk，否则会报错
- 配置环境变量：（安装完成后似乎会自动添加）
  - 系统变量中：
    - MSMPI_BENCHMARKS: path\to\mpi\Benchmarks
    - MSMPI_BIN path\to\mpi\Bin
    - MSMPI_INC path\to\mpi_sdk\Include
    - MSMPI_LIB32 path\to\mpi_sdk\Lib\x86
    - MSMPI_LIB64 path\to\mpi_sdk\Lib\x64
- cmake -B build -DBUILD_SHARED_LIBS=ON -DVTK_VERSIONED_INSTALL=OFF -DVTK_INSTALL_SDK=ON -DVTK_GROUP_ENABLE_Qt=YES -DCMAKE_BUILD_TYPE=Release -G"MinGW Makefiles" -DCMAKE_CXX_FLAGS=-fcommon -DCAMKE_C_FLAGS=-fcommon -DCMAKE_INSTALL_PREFIX=/install/dir -DVTK_MODULE_ENABLE_VTK_RenderingParallel:STRING=WANT -DVTK_USE_MPI=ON
- GLEW:-DVTK_USE_SYSTEM_GLEW=ON -DVTK_MODULE_USE_EXTERNAL_VTK_glew=ON
  - 如果编译时报错说找不到mpi可以把整个build文件夹删掉后再编译
- cmake --build build && cmake --build build --target install

## vtkCompositeRenderManager

An object to control **sort-last** parallel rendering.控制按最后排序并行呈现的对象。

uses compositing to do parallel rendering使用合成来进行并行渲染



### 实验日志：

#### 0611

指令：mpiexec -np 1000 ./multi_render_cylinders

报错：

```
x2a3347f6e90): GLEW could not be initialized.
2024-06-11 22:32:08.130 ( 148.537s) [                ]vtkWin32OpenGLRenderWin:614    ERR| vtkWin32OpenGLRenderWindow (0x21eee666e90): GLEW could not be initialized.
2024-06-11 22:32:08.129 ( 151.525s) [                ]vtkWin32OpenGLRenderWin:614    ERR| vtkWin32OpenGLRenderWindow (0x1af57636e90): GLEW could not be initialized.
2024-06-11 22:32:08.128 ( 140.971s) [                ]vtkWin32OpenGLRenderWin:614    ERR| vtkWin32OpenGLRenderWindow (0x29420b96e90): GLEW could not be initialized.
2024-06-11 22:32:08.134 ( 143.643s) [                ]vtkWin32OpenGLRenderWin:614    ERR| vtkWin32OpenGLRenderWindow (0x1a428026e90): GLEW could not be initialized.

job aborted:
[ranks] message

[0-149] terminated

[150] process exited without calling finalize

[151-999] terminated

---- error analysis -----

[150] on LAPTOP-DBFNRHRV
.\multi_render_cylinders.exe ended prematurely and may have crashed. exit code 0xc0000005
```

#### 0620

如果不用mpiexec:只有一个黑框，没有任何图形

#### 0618

- 解决方案一：https://discourse.vtk.org/t/glew-could-not-be-initialized-missing-gl-version-vtk-under-python-windows/4598/1
- vtkglew:https://github.com/Kitware/VTK/tree/master/ThirdParty/glew/vtkglew

vtk编译时加入GLEW相关参数后，在cmake --build build时报错

#### 0622

使用TaskParallism.cxx官方代码，出现报错：

```cpp
job aborted:
[ranks] message

[0] terminated

[1] process exited without calling finalize

---- error analysis -----
    [1] on LAPTOP-DBFNRHRV
./TaskParallism.exe ended prematurely and may have crashed. exit code 0xcfffffff


```

和0611后半段报错很相似

同样的问题链接：https://stackoverflow.com/questions/62304395/mpi-program-error-why-i-get-allways-this-exit-code-0xc0000005

逐行找错误方式：在每一段功能（比如创建一个renderer,renderwindow）之后打印myId

- 结果：可以打印出0,1（两个进程），但仍然无显示
- 放在tc->StartInteractor();后面无法打印出来
- 删除这一行之后之后没有窗口了

#### 0623

SMP框架文档介绍：https://docs.vtk.org/en/latest/design_documents/smptools.html

- 不用：4.26783 3.48105 3.53558 4.3999 3.71715 3.92163 3.70983 3.79125 3.74665 3.77642
- 用了：3.71567 3.3437 3.51012 3.88117 3.62292 3.47294 3.7053 3.59275 3.73682 3.66076

## vtkImageRenderManager

An object to control **sort-first** parallel rendering.

uses RGBA compositing (blending) to do parallel rendering. This is the exact opposite of [vtkCompositeRenderManager](https://vtk.org/doc/nightly/html/classvtkCompositeRenderManager.html). It actually does nothing special. It relies on the rendering pipeline to be initialized with a [vtkCompositeRGBAPass](https://vtk.org/doc/nightly/html/classvtkCompositeRGBAPass.html). Compositing makes sense only for renderers in layer 0.使用RGBA合成(混合)进行并行渲染。这与vtkCompositeRenderManager完全相反。它实际上没有什么特别的作用。它依赖于用vtkCompositeRGBAPass初始化的渲染管道。合成只对第0层的渲染器有意义。

**vtkCompositeRGBAPass:**

Blend RGBA buffers of processes.

Blend the RGBA buffers of satellite processes over the root process RGBA buffer. The RGBA buffer of the satellite processes are not changed.

This pass requires a OpenGL context that supports texture objects (TO), and pixel buffer objects (PBO). If not, it will emit an error message and will render its delegate and return.

混合进程的RGBA缓冲区。

将卫星进程的RGBA缓冲区混合到根进程的RGBA缓冲区上。不改变卫星进程的RGBA缓冲区。

这个通道需要一个支持纹理对象(TO)和像素缓冲对象(PBO)的OpenGL上下文。如果没有，它将发出一条错误消息，并呈现它的委托并返回。

多进程：多次执行同一个可执行文件（mpi）

多线程：把一个进程分成多个部分来并行执行