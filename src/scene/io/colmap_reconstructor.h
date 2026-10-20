#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>

namespace colmap
{
    class AutomaticReconstructionController;
    class ReconstructionManager;
}

namespace tri
{
    enum class ColmapQuality
    {
        Low,
        Medium,
        High,
        Extreme
    };

    enum class ColmapStatus
    {
        Idle,
        Running,
        Success,
        Failed,
        Cancelled
    };

    class ColmapReconstructor
    {
    public:
        ColmapReconstructor();
        ~ColmapReconstructor();

        // Starts dense reconstruction asynchronously using linked COLMAP C++ library
        bool Start(const std::string& input_dir, ColmapQuality quality = ColmapQuality::High);

        // Cancels running reconstruction
        void Cancel();

        // Resets state to Idle
        void Reset();

        bool IsRunning() const { return is_running_; }
        ColmapStatus GetStatus() const { return status_; }
        const std::string& GetStatusMessage() const;
        const std::string& GetFusedPlyPath() const;
        const std::string& GetInputDir() const { return input_dir_; }

        // Returns all log messages so far
        std::vector<std::string> GetLogs();
        size_t GetLogCount();

    private:
        void RunWorker(std::string input_dir, ColmapQuality quality);
        std::string FindFusedPly(const std::string& workspace_dir);
        std::string FindSystemColmap() const;
        bool RunDenseWithSystemColmap(const std::string& colmap_exe, const std::string& workspace_dir, const std::string& image_dir, ColmapQuality quality);
        void AddLog(const std::string& message);

        std::atomic<bool> is_running_{ false };
        std::atomic<bool> cancel_requested_{ false };
        std::atomic<ColmapStatus> status_{ ColmapStatus::Idle };

        std::string input_dir_;
        std::string workspace_dir_;
        std::string fused_ply_path_;
        std::string status_message_;

        std::thread worker_thread_;
        std::mutex mutex_;
        std::vector<std::string> logs_;

        std::shared_ptr<colmap::AutomaticReconstructionController> controller_;
    };
} // namespace tri
