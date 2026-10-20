#include "scene/io/colmap_reconstructor.h"
#include "core/logger.h"
#include <colmap/controllers/automatic_reconstruction.h>
#include <colmap/scene/reconstruction_manager.h>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

using namespace tri;

ColmapReconstructor::ColmapReconstructor()
{
}

ColmapReconstructor::~ColmapReconstructor()
{
    Cancel();
    if (worker_thread_.joinable())
    {
        worker_thread_.join();
    }
}

void ColmapReconstructor::AddLog(const std::string& message)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.push_back(message);
    }
    TRI_LOG_INFO("COLMAP", message);
}

std::vector<std::string> ColmapReconstructor::GetLogs()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return logs_;
}

size_t ColmapReconstructor::GetLogCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return logs_.size();
}

const std::string& ColmapReconstructor::GetStatusMessage() const
{
    return status_message_;
}

const std::string& ColmapReconstructor::GetFusedPlyPath() const
{
    return fused_ply_path_;
}

void ColmapReconstructor::Reset()
{
    if (is_running_)
    {
        return;
    }
    status_ = ColmapStatus::Idle;
    status_message_.clear();
    fused_ply_path_.clear();
    input_dir_.clear();
    workspace_dir_.clear();
    std::lock_guard<std::mutex> lock(mutex_);
    logs_.clear();
}

void ColmapReconstructor::Cancel()
{
    if (!is_running_)
    {
        return;
    }
    cancel_requested_ = true;
    std::lock_guard<std::mutex> lock(mutex_);
    if (controller_)
    {
        controller_->Stop();
    }
}

bool ColmapReconstructor::Start(const std::string& input_dir, ColmapQuality quality)
{
    if (is_running_)
    {
        return false;
    }

    if (worker_thread_.joinable())
    {
        worker_thread_.join();
    }

    input_dir_ = input_dir;
    cancel_requested_ = false;
    is_running_ = true;
    status_ = ColmapStatus::Running;
    status_message_ = "Starting reconstruction...";
    fused_ply_path_.clear();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.clear();
    }

    worker_thread_ = std::thread(&ColmapReconstructor::RunWorker, this, input_dir, quality);
    return true;
}

std::string ColmapReconstructor::FindFusedPly(const std::string& workspace_dir)
{
    namespace fs = std::filesystem;
    fs::path ws(workspace_dir);

    // Primary candidates
    std::vector<fs::path> candidates = {
        ws / "dense" / "0" / "fused.ply",
        ws / "dense" / "fused.ply",
        ws / "dense" / "0" / "meshed-poisson.ply",
        ws / "dense" / "meshed-poisson.ply"
    };

    for (const auto& candidate : candidates)
    {
        if (fs::exists(candidate))
        {
            return candidate.string();
        }
    }

    // Search dense subdirectories for any ply file
    fs::path dense_dir = ws / "dense";
    if (fs::exists(dense_dir) && fs::is_directory(dense_dir))
    {
        for (const auto& entry : fs::recursive_directory_iterator(dense_dir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".ply")
            {
                return entry.path().string();
            }
        }
    }

    return "";
}

std::string ColmapReconstructor::FindSystemColmap() const
{
    namespace fs = std::filesystem;

    const char* env_path = std::getenv("COLMAP_EXE");
    if (env_path && fs::exists(env_path))
    {
        return env_path;
    }

    std::vector<std::string> candidates = {
        "D:\\app\\colmap\\v4.1.1-cuda\\COLMAP.bat",
        "C:\\Program Files\\COLMAP\\COLMAP.bat",
        "D:\\COLMAP\\COLMAP.bat",
        "C:\\COLMAP\\COLMAP.bat"
    };

    for (const auto& path : candidates)
    {
        if (fs::exists(path))
        {
            return path;
        }
    }

    return "";
}

bool ColmapReconstructor::RunDenseWithSystemColmap(const std::string& colmap_exe, const std::string& workspace_dir, const std::string& image_dir, ColmapQuality quality)
{
    namespace fs = std::filesystem;
    std::string quality_str = "high";
    switch (quality)
    {
        case ColmapQuality::Low: quality_str = "low"; break;
        case ColmapQuality::Medium: quality_str = "medium"; break;
        case ColmapQuality::High: quality_str = "high"; break;
        case ColmapQuality::Extreme: quality_str = "extreme"; break;
    }

    std::string cmd = "\"" + colmap_exe + "\" automatic_reconstructor"
        + " --workspace_path \"" + workspace_dir + "\""
        + " --image_path \"" + image_dir + "\""
        + " --quality " + quality_str
        + " --dense 1";

    AddLog("Running CUDA dense reconstruction pipeline...");

#if defined(_WIN32)
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE read_pipe = nullptr;
    HANDLE write_pipe = nullptr;
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0))
    {
        AddLog("Failed to create pipe for COLMAP process.");
        return false;
    }
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = write_pipe;
    si.hStdError = write_pipe;
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::vector<char> cmd_buf(cmd.begin(), cmd.end());
    cmd_buf.push_back('\0');

    if (!CreateProcessA(nullptr, cmd_buf.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        AddLog("Failed to spawn COLMAP process.");
        return false;
    }

    CloseHandle(write_pipe);

    char buffer[512];
    DWORD bytes_read = 0;
    std::string current_line;

    while (ReadFile(read_pipe, buffer, sizeof(buffer) - 1, &bytes_read, nullptr) && bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        for (DWORD i = 0; i < bytes_read; ++i)
        {
            char c = buffer[i];
            if (c == '\r') continue;
            if (c == '\n')
            {
                if (!current_line.empty())
                {
                    AddLog(current_line);
                    current_line.clear();
                }
            }
            else
            {
                current_line += c;
            }
        }

        if (cancel_requested_)
        {
            TerminateProcess(pi.hProcess, 1);
            break;
        }
    }

    if (!current_line.empty())
    {
        AddLog(current_line);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(read_pipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exit_code == 0;
#else
    return false;
#endif
}

void ColmapReconstructor::RunWorker(std::string input_dir, ColmapQuality quality)
{
    namespace fs = std::filesystem;

    try
    {
        AddLog("Initializing COLMAP C++ reconstruction pipeline...");

        fs::path input_path(input_dir);
        if (!fs::exists(input_path))
        {
            status_ = ColmapStatus::Failed;
            status_message_ = "Input path does not exist: " + input_dir;
            AddLog("Error: " + status_message_);
            is_running_ = false;
            return;
        }

        fs::path image_path = input_path;
        if (fs::is_directory(input_path / "images"))
        {
            image_path = input_path / "images";
        }

        fs::path workspace_path = input_path / "colmap_workspace";
        fs::create_directories(workspace_path);
        workspace_dir_ = workspace_path.string();

        AddLog("Image path: " + image_path.string());
        AddLog("Workspace path: " + workspace_path.string());

        // Check if dense point cloud already exists in workspace
        std::string existing_ply = FindFusedPly(workspace_dir_);
        if (!existing_ply.empty())
        {
            fused_ply_path_ = existing_ply;
            status_ = ColmapStatus::Success;
            status_message_ = "Dense point cloud already reconstructed!";
            AddLog("Found existing dense point cloud: " + existing_ply);
            is_running_ = false;
            return;
        }

        colmap::AutomaticReconstructionController::Options options;
        options.workspace_path = workspace_path.string();
        options.image_path = image_path.string();
        options.dense = true; // Request dense mapping
        options.use_gpu = false; // CPU feature extraction
        options.num_threads = 1; // Single thread to avoid multi-thread deadlock in CPU feature matching

        switch (quality)
        {
            case ColmapQuality::Low:
                options.quality = colmap::AutomaticReconstructionController::Quality::LOW;
                AddLog("Quality setting: LOW");
                break;
            case ColmapQuality::Medium:
                options.quality = colmap::AutomaticReconstructionController::Quality::MEDIUM;
                AddLog("Quality setting: MEDIUM");
                break;
            case ColmapQuality::High:
                options.quality = colmap::AutomaticReconstructionController::Quality::HIGH;
                AddLog("Quality setting: HIGH");
                break;
            case ColmapQuality::Extreme:
                options.quality = colmap::AutomaticReconstructionController::Quality::EXTREME;
                AddLog("Quality setting: EXTREME");
                break;
        }

        auto reconstruction_manager = std::make_shared<colmap::ReconstructionManager>();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            controller_ = std::make_shared<colmap::AutomaticReconstructionController>(options, reconstruction_manager);
        }

        AddLog("Starting linked COLMAP C++ automatic reconstruction controller...");
        status_message_ = "Reconstruction in progress...";

        // Execute reconstruction via linked C++ library
        controller_->Start();
        controller_->Wait();

        if (cancel_requested_)
        {
            status_ = ColmapStatus::Cancelled;
            status_message_ = "Reconstruction was cancelled.";
            AddLog(status_message_);
            is_running_ = false;
            return;
        }

        AddLog("COLMAP C++ library pipeline completed. Checking for dense point cloud...");
        std::string ply_path = FindFusedPly(workspace_dir_);

        // If dense reconstruction was skipped (due to no CUDA support in linked library build),
        // complement with system CUDA-enabled COLMAP pipeline
        if (ply_path.empty())
        {
            AddLog("Dense point cloud not produced by CPU library. Searching for CUDA-enabled COLMAP...");
            std::string system_colmap = FindSystemColmap();
            if (!system_colmap.empty())
            {
                AddLog("Found CUDA-enabled COLMAP engine: " + system_colmap);
                status_message_ = "Running CUDA dense stereo pipeline...";
                RunDenseWithSystemColmap(system_colmap, workspace_dir_, image_path.string(), quality);
                ply_path = FindFusedPly(workspace_dir_);
            }
        }

        if (!ply_path.empty())
        {
            fused_ply_path_ = ply_path;
            status_ = ColmapStatus::Success;
            status_message_ = "Dense point cloud successfully generated!";
            AddLog("Found dense point cloud: " + ply_path);
        }
        else
        {
            // If dense reconstruction failed or was not generated, check for sparse model points3D
            fs::path sparse_path = workspace_path / "sparse" / "0";
            if (fs::exists(sparse_path / "points3D.bin") || fs::exists(sparse_path / "points3D.txt"))
            {
                fused_ply_path_ = sparse_path.string();
                status_ = ColmapStatus::Success;
                status_message_ = "Dense ply not found, but sparse model was generated.";
                AddLog("Fallback to sparse model: " + sparse_path.string());
            }
            else
            {
                status_ = ColmapStatus::Failed;
                status_message_ = "No reconstructed model or dense point cloud found in workspace.";
                AddLog("Error: " + status_message_);
            }
        }
    }
    catch (const std::exception& e)
    {
        status_ = ColmapStatus::Failed;
        status_message_ = std::string("Exception occurred: ") + e.what();
        AddLog("Error: " + status_message_);
    }
    catch (...)
    {
        status_ = ColmapStatus::Failed;
        status_message_ = "Unknown error occurred during COLMAP reconstruction.";
        AddLog("Error: " + status_message_);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        controller_.reset();
    }
    is_running_ = false;
}
