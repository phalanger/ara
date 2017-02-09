#ifndef ARA_LOG_APPENDER_ROLLINGFILE_H_201701
#define ARA_LOG_APPENDER_ROLLINGFILE_H_201701

#include "log_imp.h"
#include "raw_file.h"

#include "../filesys.h"
#include "../datetime.h"

namespace ara {
	namespace log {
		class appender_rollingfile : public appender
		{
		public:
			appender_rollingfile(const std::string & file
				, const timer_val & rolltime = 1_day
				, size_t	max_file_size = 0
				, size_t	keep_file_count = 0
			) 	: file_name_(file_sys::fix_path(file))
				, roll_time_(rolltime)
				, max_file_size_(max_file_size)
				, keep_file_count_(keep_file_count) 
			{}

			virtual bool		before_write(const log_data & data, std::ostream & out) {
				stream_printf(out).printf("T:%v(%v)[%v:%v] "
					, data.thread_id()
					, data.log_time().format("%H:%M:%S")
					, data.get_logger().get_name()
					, log_data::get_level_name(data.get_level())
				);
				return true;
			}
			virtual bool		on_flush(const log_data & data, const ref_string & content) {
				std::lock_guard<std::mutex>		_guard(lock_);
				check_openfile(data.log_time());
				if (rf_.is_opened()) {
					current_size_ += content.size();
					rf_.write(content.data(), content.size());
				}
				return true;
			}

			virtual std::string	dump_setting() const {
				return str_printf<std::string>("rollingfile_appender(n:%v t:%v, s:%v, c:%v)"
					, file_name_
					, roll_time_
					, max_file_size_
					, keep_file_count_
					);
			}

			virtual void	flush_all() {
			}

		protected:
			void		check_openfile(const date_time & logtime) {
				if (logtime < next_roll_time_ && (current_size_ < max_file_size_ - 1))
					return;
				if (next_roll_time_.empty())
					check_history();
				auto now = date_time::get_current();
				next_roll_time_ = now;
				if (roll_time_ >= 1_day)
					next_roll_time_.set_time(0, 0, 0);
				else {
					auto t = next_roll_time_.get();
					next_roll_time_.set(t - t % roll_time_.sec());
				}
				next_roll_time_.step( 0, 0 , 0, 0, 0, static_cast<int>(roll_time_.sec()));
				
				std::string strFileName = file_name_;

				if (roll_time_ >= 1_day) {

					strFileName += now.format(".%Y-%m-%d");

					if (strFileName == current_filename_) //that means size too large
						strFileName += now.format(".%H%M%S");
					else
						current_filename_ = strFileName;
				}
				else
					strFileName += now.format(".%Y-%m-%d.%H%M%S");

				rf_.open(strFileName).create().write_only().append().done();
				current_size_ = 0;
				

#ifndef ARA_WIN32_VER
				struct stat statbuf;
				memset(&statbuf, 0, sizeof(statbuf));
				if (::lstat(file_name_.c_str(), &statbuf) != 0 || (statbuf.st_mode & S_IFLNK) != 0)
				{
					::unlink(file_name_.c_str());
					(void)::symlink(strFileName.c_str(), file_name_.c_str());
				}
#endif
			}
			void		check_history() {
				if (keep_file_count_ == 0)
					return;
				std::string sPath, sFile;
				file_sys::split_path(file_name_, sPath, sFile);
				std::vector<std::string>		vectPathName;
				for (auto it : scan_dir(sPath)) {
					if (it.find(sFile) == 0)
						vectPathName.push_back(it);
				}
				if (vectPathName.size() < keep_file_count_)
					return;
				std::sort(vectPathName.begin(), vectPathName.end());
				vectPathName.resize(vectPathName.size() - keep_file_count_ + 1);
				for (auto n : vectPathName) {
					file_sys::unlink(file_sys::join_to_file(sPath, n));
				}
			}

			std::mutex		lock_;
			std::string		file_name_;
			date_time		next_roll_time_;
			timer_val		roll_time_;
			size_t			max_file_size_ = 0;
			size_t			keep_file_count_ = 0;
			size_t			current_size_ = 0;
			std::string		current_filename_;
			ara::raw_file	rf_;
		};
	}
}

#endif//ARA_LOG_APPENDER_ROLLINGFILE_H_201701
