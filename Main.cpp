# include <Siv3D.hpp>
# include <Windows.h>

struct testcase_result {
	String testcase_name = U"";
	String result = U"";
	String runtime = 0;
};

struct Judge_info {
	String submission_ID = U"0";
	String submission_time = U"";
	String contest_name = U"";
	String task_name = U"";
	String testcase_path = U"";
	String code_path = U"";
	String result = U"WJ";
	String error_code = U"";
	String lang = U"";
	Array<testcase_result>testcases_status;
	int testcases_num = 0;
	int judged_testcases_num = 0;
	int time_limit = 2;
	double max_runtime = 0;
};

Judge_info status;

struct Judge_result {
	String submission_ID = U"";
	DateTime submission_time = DateTime{ 1970,1,1 } + 9h;
	String contest_name = U"";
	String task_name = U"";
	String result = U"";
	String lang = U"";
	double runtime;
};

bool stop = false;

void call_python(LPSTR path) {
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	memset(&si, 0, sizeof(STARTUPINFOA));
	si.cb = sizeof(STARTUPINFOA);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	CreateProcessA(NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

bool compile_cpp() {
	TextWriter{ U"data/judge/main.cpp" }.write(TextReader{ status.code_path }.readAll());
	call_python((LPSTR)"python data/compile.py");
	FileSystem::Copy(U"data/judge/compiler_out.txt", U"data/code/" + status.submission_ID + U"/compiler_out.txt");
	String compile_result;
	JSON json = JSON::Load(U"data/judge/judge.json");
	compile_result = json[U"compile"].getString();
	json.save(U"data/judge/judge.json");
	JSON result = JSON::Load(U"data/code/" + status.submission_ID + U"/result.json");
	if (compile_result != U"0") {
		status.error_code = U"CE";
		result[U"result"] = U"CE";
		result.save(U"data/code/" + status.submission_ID + U"/result.json");
		return false;
	}
	return true;
}

void Judge_testcase(LPSTR command) {
	JSON judge_json = JSON::Load(U"data/judge/judge.json");
	judge_json[U"TL"] = ToString(status.time_limit);
	judge_json.save(U"data/judge/judge.json");
	JSON result = JSON::Load(U"data/code/" + status.submission_ID + U"/result.json");
	bool flag_ac = true, flag_tle = false, flag_re = false;
	for (auto& testcase : status.testcases_status) {
		if (stop) {
			status.error_code = U"Cancelled";
			result.save(U"data/code/" + status.submission_ID + U"/result.json");
			return;
		}
		judge_json[U"task-input"] = status.testcase_path + U"/in/" + testcase.testcase_name + U".txt";
		judge_json[U"task-output"] = status.testcase_path + U"/out/" + testcase.testcase_name + U".txt";
		judge_json.save(U"data/judge/judge.json");
		call_python(command);
		judge_json = JSON::Load(U"data/judge/judge.json");
		status.max_runtime = std::max(status.max_runtime, Parse<double>(judge_json[U"runtime"].getString()));
		result[U"testcases"][testcase.testcase_name][U"runtime"] = judge_json[U"runtime"];
		testcase.runtime = judge_json[U"runtime"].getString();
		if (judge_json[U"result"] != U"JUDGE") {
			flag_ac = false;
			if (judge_json[U"result"] == U"RE")flag_re = true;
			if (judge_json[U"result"] == U"TLE")flag_tle = true;
		}
		else {
			call_python((LPSTR)"python data/judge.py");
			judge_json = JSON::Load(U"data/judge/judge.json");
			if (judge_json[U"result"].getString() != U"AC") {
				flag_ac = false;
			}
		}
		testcase.result = judge_json[U"result"].getString();
		result[U"testcases"][testcase.testcase_name][U"result"] = judge_json[U"result"];
		if (flag_ac) {
			status.result = U"WJ";
		}
		else if (flag_re) {
			status.result = U"RE";
		}
		else if (flag_tle) {
			status.result = U"TLE";
		}
		else {
			status.result = U"WA";
		}
		status.judged_testcases_num++;
	}
	if (flag_ac)status.result = U"AC";
	result.save(U"data/code/" + status.submission_ID + U"/result.json");
}

Judge_result make_result() {
	Judge_result temp{
		status.submission_ID,
		DateTime{ 1970,1,1 } + 9h + (std::chrono::seconds)Parse<int>(status.submission_time),
		status.contest_name,
		status.task_name,
		(status.error_code == U"") ? status.result : status.error_code,
		status.lang,
		status.max_runtime

	};
	JSON result = JSON::Load(U"data/code/" + status.submission_ID + U"/result.json");
	result[U"result"] = temp.result;
	result[U"runtime"] = Format(temp.runtime);
	result.save(U"data/code/" + status.submission_ID + U"/result.json");
	return temp;
}

void Init_json_result() {
	JSON result;
	result[U"submission_ID"] = status.submission_ID;
	result[U"submission_time"] = status.submission_time;
	result[U"contest_name"] = status.contest_name;
	result[U"task_name"] = status.task_name;
	result[U"result"] = U"WJ";
	result[U"lang"] = status.lang;
	result[U"runtime"] = U"0";
	result[U"testcases"];
	result.save(U"data/code/" + status.submission_ID + U"/result.json");
}

void Init_json_judge() {
	JSON judge;
	judge[U"compile"] = U"";
	judge[U"TL"] = U"2";
	judge[U"runtime"] = U"0";
	judge[U"result"] = U"";
	judge.save(U"data/judge/judge.json");
}

void Judge() {
	Init_json_result();
	Init_json_judge();
	if (FileSystem::IsDirectory(status.testcase_path + U"/in") && FileSystem::IsDirectory(status.testcase_path + U"/out")) {
		std::set<String>task_in, task_out;
		for (const auto& search_path : FileSystem::DirectoryContents(status.testcase_path + U"/in", Recursive::No)) {
			task_in.insert(FileSystem::BaseName(search_path));
		}
		for (const auto& search_path : FileSystem::DirectoryContents(status.testcase_path + U"/out", Recursive::No)) {
			task_out.insert(FileSystem::BaseName(search_path));
		}
		for (const auto& testcase_name : task_in) {
			if (task_out.count(testcase_name)) {
				status.testcases_status.push_back({ testcase_name, U"",U"" });
			}
		}
		status.testcases_num = status.testcases_status.size();
		if (status.testcases_status.size() == 0) {
			status.error_code = U"NOTASK";
			make_result();
			return;
		}
	}
	else {
		status.error_code = U"NOTASK";
		return;
	}
	String code_path = U"data/code/" + status.submission_ID + U"/main";
	if (FileSystem::IsFile(code_path + U".cpp")) {
		status.code_path = code_path + U".cpp";
		if (compile_cpp()) {
			Judge_testcase((LPSTR)"python data/cpp.py");
		}
	}
	else if (FileSystem::IsFile(code_path + U".py")) {
		status.code_path = code_path + U".py";
		Judge_testcase((LPSTR)"python data/py.py");
	}
	else {
		status.error_code = U"NOCODE";
	}
	make_result();
}

std::map<String, Array<String>> search_task() {
	std::map<String, Array<String>>tasks;
	for (const auto& contest_path : FileSystem::DirectoryContents(U"data/contests", Recursive::No)) {
		if (FileSystem::IsDirectory(contest_path)) {
			String contest_name = FileSystem::BaseName(contest_path);
			for (const auto& testcase_path : FileSystem::DirectoryContents(contest_path, Recursive::No)) {
				if (FileSystem::IsDirectory(testcase_path)) {
					tasks[contest_name].push_back(FileSystem::BaseName(testcase_path));
				}
			}
		}
	}
	return tasks;
}

void Main() {
	//Settings
	Window::SetTitle(U"Offline Judge");
	Window::Resize(1280, 720);
	Scene::SetBackground(Palette::Whitesmoke);

	//Textures,Fonts
	const Texture sub_tex{ 0xf1d8_icon,30 };
	const Texture res_tex{ 0xf187_icon,30 };
	const Font sfont{ FontMethod::MSDF, 18 };
	const Font nfont{ FontMethod::MSDF, 30 };

	//Judge
	std::map<String, Array<String>>tasks;
	size_t select_lang = 0;
	Array<Judge_info>waiting_judge;
	AsyncTask<void> judge_task;
	int latest_submission_ID = 0;

	//GUI
	String mode = U"Submit", results_mode = U"ALL";
	int current_page = 0, results_pages = 0;
	int scroll_detail = 0;
	ListBoxState contest_list, task_list;
	TextAreaEditState submit_code;
	TextAreaEditState detail_code, detail_compiler;
	TextEditState time_limit{ U"2" };
	Judge_result submission_detail;
	Array<Judge_result> result_array;
	SimpleTable submit_table{ {225,175,110,110,140,110,100},{.fontSize = 22} };
	SimpleTable testcase_table{ {200,120,120},{.fontSize = 22 } };
	SimpleTable detail_table{ {235,235},{.fontSize = 22} };

	//Init
	tasks = search_task();
	for (auto i : FileSystem::DirectoryContents(U"data/code", Recursive::No)) {
		if (FileSystem::IsDirectory(i) == false)continue;
		if (FileSystem::IsFile(i + U"/result.json")) {
			Judge_result temp;
			JSON json = JSON::Load(i + U"/result.json");
			temp.submission_ID = FileSystem::BaseName(i);
			temp.contest_name = json[U"contest_name"].getString();
			temp.task_name = json[U"task_name"].getString();
			temp.contest_name = json[U"contest_name"].getString();
			if (json[U"result"].getString() == U"WJ") {
				json[U"result"] = U"Canceled";
				json.save(i + U"/result.json");
			}
			temp.result = json[U"result"].getString();
			temp.lang = json[U"lang"].getString();
			temp.submission_time += (std::chrono::seconds)Parse<int>(json[U"submission_time"].getString());
			try {
				temp.runtime = Parse<double>(json[U"runtime"].getString());
			}
			catch (const ParseError& e) {
				temp.runtime = 0;
			}
			result_array.push_back(temp);
		}
		try {
			latest_submission_ID = std::max(latest_submission_ID, Parse<int>(FileSystem::BaseName(i)));
		}
		catch (const ParseError& e) {}
	}
	result_array.sort_by([](const Judge_result& a, const Judge_result& b) {return a.submission_time < b.submission_time; });
	for (auto i : tasks) {
		contest_list.items << i.first;
	}
	if (tasks.size() != 0) {
		contest_list.selectedItemIndex = 0;
		for (auto i : tasks[contest_list.items[*contest_list.selectedItemIndex]]) {
			task_list.items << i;
		}
		task_list.selectedItemIndex = 0;
	}
	submit_table.push_back_row({ U"Submission time",U"Contest",U"Task",U"Lang",U"Status",U"Runtime",U"" });
	for (int i = result_array.size() - 1; i >= std::max(((int)result_array.size()) - 16, 0); i--) {
		submit_table.push_back_row({
			result_array[i].submission_time.format(),
			result_array[i].contest_name,
			result_array[i].task_name,
			U"Lang",
			result_array[i].result,
			Format(result_array[i].runtime),
			U"Detail" });
	}
	results_pages = ((int)result_array.size()) / 16;

	while (System::Update()) {
		//Judge update
		{
			if (judge_task.isValid()) {
				for (int i = result_array.size() - 1; i >= 0; i--) {
					if (result_array[i].submission_ID == status.submission_ID) {
						result_array[i].result = status.result + U"(" + Format(status.judged_testcases_num) + U"/" + Format(status.testcases_num) + U")";
						break;
					}
				}
				if (submission_detail.submission_ID == status.submission_ID) {
					submission_detail.result = status.result + U"(" + Format(status.judged_testcases_num) + U"/" + Format(status.testcases_num) + U")";
				}
			}
			if (waiting_judge.size() != 0) {
				if (judge_task.isReady()) {
					judge_task.get();
					for (int i = result_array.size() - 1; i >= 0; i--) {
						if (result_array[i].submission_ID == status.submission_ID) {
							result_array[i] = make_result();
							break;
						}
					}
					if (submission_detail.submission_ID == status.submission_ID)submission_detail.result = status.result;
					status = waiting_judge[0];
					waiting_judge.erase(waiting_judge.begin());
					stop = false;
					judge_task = Async(Judge);
				}
				else if (judge_task.isValid() == false) {
					status = waiting_judge[0];
					waiting_judge.erase(waiting_judge.begin());
					stop = false;
					judge_task = Async(Judge);
				}
			}
			else if (judge_task.isReady()) {
				judge_task.get();
				for (int i = result_array.size() - 1; i >= 0; i--) {
					if (result_array[i].submission_ID == status.submission_ID) {
						result_array[i] = make_result();
						break;
					}
					if (submission_detail.submission_ID == status.submission_ID)submission_detail.result = status.result;
				}
			}
		}
		//Menu
		{
			Rect{ 0, 0, 240,720 }.draw(Palette::Lightgray);
			Line{ 0,0,1280,0 }.draw(Palette::Gray);
			if (mode == U"Results") {
				if (sub_tex.drawAt(40, 60, Palette::Gray).leftClicked()) {
					mode = U"Submit";
				}
				if (nfont(U"Submit").drawAt(120, 60, Palette::Gray).stretched(20, 20).leftClicked()) {
					mode = U"Submit";
				}
				if (nfont(U"Results").drawAt(125, 140, ColorF{ 0.0 }).leftClicked()) {
					results_mode = U"ALL";
				}
				if (res_tex.drawAt(40, 140, Palette::Black).leftClicked()) {
					results_mode = U"ALL";
				}
			}
			else {
				if (res_tex.drawAt(40, 140, Palette::Gray).leftClicked()) {
					mode = U"Results";
					results_mode = U"ALL";
				}
				if (nfont(U"Results").drawAt(125, 140, Palette::Gray).stretched(20, 20).leftClicked()) {
					mode = U"Results";
					results_mode = U"ALL";
				}
				sub_tex.drawAt(40, 60, Palette::Black);
				nfont(U"Submit").drawAt(120, 60, ColorF{ 0.0 });
			}
			if (SimpleGUI::Button(U"Cancel running judge", { 10,670 }, 220, judge_task.isValid())) {
				stop = true;
			}
		}
		if (mode == U"Submit") {
			//Font
			{
				nfont(U"Contest").drawAt(342, 50, ColorF{ 0.0 });
				nfont(U"Task").drawAt(538, 50, ColorF{ 0.0 });
				nfont(U"Code").drawAt(755, 50, ColorF{ 0.0 });
				nfont(U"Lang").drawAt(1116, 50, ColorF{ 0.0 });
				nfont(U"TL").drawAt(1100, 200, ColorF{ 0.0 });
			}
			//GUI
			{
				if (SimpleGUI::ListBox(contest_list, Vec2{ 280,70 }, 180, 610)) {
					task_list.items.clear();
					for (auto i : tasks[contest_list.items[*contest_list.selectedItemIndex]]) {
						task_list.items << i;
					}
				}
				SimpleGUI::ListBox(task_list, Vec2{ 500,70 }, 180, 610);
				SimpleGUI::TextArea(submit_code, Vec2{ 720,70 }, SizeF{ 340, 610 }, 100000);
				SimpleGUI::RadioButtons(select_lang, { U"C++",U"Python" }, Vec2{ 1080,70 }, 165);
				Rect{ 1080,70,165,80 }.drawFrame(1, Palette::Gray);
				SimpleGUI::TextBox(time_limit, Vec2{ 1080,220 }, 165, 2);
				if (SimpleGUI::Button(U"Copy", Vec2{ 1080,280 }, 165)) {
					Clipboard::SetText(submit_code.text);
				}
				if (SimpleGUI::Button(U"Clear", Vec2{ 1080,350 }, 165)) {
					submit_code.clear();
				}
			}
			//Edit textbox
			{
				if (time_limit.text != U"") {
					try {
						unsigned int temp = Parse<unsigned int>(time_limit.text);
						if (temp > 10)temp = 10;
						time_limit.text = Format(temp);
					}
					catch (const ParseError& e) {
						time_limit.text = U"2";
					}
				}
				if (time_limit.active == false && time_limit.text == U"") {
					time_limit.text = U"2";
				}
			}
			//Submit
			if (SimpleGUI::Button(U"Submit", Vec2{ 1080,640 }, 165, waiting_judge.size() <= 5)) {
				if (tasks.size() != 0) {
					FileSystem::CreateDirectories(U"data/code/" + ToString(latest_submission_ID + 1));
					if (select_lang == 0) {
						FileSystem::Copy(U"data/copy/main.cpp", U"data/code/" + ToString(latest_submission_ID + 1) + U"/main.cpp");
						TextWriter newcode{ U"data/code/" + ToString(latest_submission_ID + 1) + U"/main.cpp" };
						newcode << submit_code.text;
					}
					else {
						FileSystem::Copy(U"data/copy/main.py", U"data/code/" + ToString(latest_submission_ID + 1) + U"/main.py");
						TextWriter newcode{ U"data/code/" + ToString(latest_submission_ID + 1) + U"/main.py" };
						newcode << submit_code.text;
					}
					Judge_info newsubmit{
						ToString(latest_submission_ID + 1),
						Format(Time::GetSecSinceEpoch()),
						contest_list.items[*contest_list.selectedItemIndex],
						task_list.items[*task_list.selectedItemIndex],
						U"data/contests/" + contest_list.items[*contest_list.selectedItemIndex] + U"/" + task_list.items[*task_list.selectedItemIndex],
						U"",
						U"WJ",
						U"",
						Array<String>{ U"C++",U"Python" }[select_lang],
						Array<testcase_result>{},
						0,
						0,
						Parse<int>(time_limit.text),
						0
					};
					waiting_judge.push_back(newsubmit);
					result_array.push_back({
						Format(latest_submission_ID + 1),
						DateTime{1970,1,1} + (std::chrono::seconds)Parse<int>(newsubmit.submission_time),
						newsubmit.contest_name,
						newsubmit.task_name,
						U"WJ",
						Array<String>{U"C++",U"Python"}[select_lang],
						0.0
					});
					results_pages = ((int)result_array.size()) / 16;
					latest_submission_ID++;
					mode = U"Results";
					results_mode = U"ALL";
				}
			}
		}
		if (mode == U"Results") {
			if (results_mode == U"ALL") {
				submit_table.draw({ 280,40 });
				if (SimpleGUI::Button(U"Prev", Vec2{ 560,665 }, 100, current_page != 0)) {
					current_page--;
				}
				if (SimpleGUI::Button(U"Next", Vec2{ 860,665 }, 100, current_page != results_pages)) {
					current_page++;
				}
				submit_table.clear();
				submit_table.push_back_row({ U"Submission time",U"Contest",U"Task",U"Lang",U"Status",U"Runtime",U"" });
				for (int i = result_array.size() - 1 - 16 * current_page; i >= std::max(((int)result_array.size()) - 16 * (current_page + 1), 0); i--) {
					submit_table.push_back_row({
						result_array[i].submission_time.format(),
						result_array[i].contest_name,
						result_array[i].task_name,
						result_array[i].lang,
						result_array[i].result,
						Format(result_array[i].runtime),
						U"Detail" });
				}
				for (int i = 1; i < submit_table.rows(); i++) {
					if (i % 2 == 0) {
						for (int j = 0; j < submit_table.columns(); j++) {
							submit_table.setBackgroundColor(i, j, Palette::Whitesmoke);
						}
					}
					submit_table.setTextColor(i, 6, Color{ 26, 13, 171 });
					String temp = submit_table.getItem(i, 4).text;
					if (temp.includes(U"AC")) {
						submit_table.setBackgroundColor(i, 4, Palette::Limegreen);
					}
					else if (temp.includes(U"WJ")) {
						submit_table.setBackgroundColor(i, 4, Palette::Gray);
					}
					else {
						submit_table.setBackgroundColor(i, 4, Palette::Goldenrod);
					}
				}
				if (result_array.size() == 0) {
					nfont(U"No Submissions").drawAt(760, 360, ColorF{ 0.0 });
				}
				nfont(Format(current_page + 1) + U"/" + Format(results_pages + 1)).drawAt(760, 685, ColorF{ 0.0 });

				if (MouseL.down()) {
					const auto detail = submit_table.cellIndex({ 280,40 }, Cursor::Pos());
					if (detail) {
						if (detail.value().x == 6 && detail.value().y > 0) {
							scroll_detail = 0;
							testcase_table.clear();
							detail_table.clear();
							detail_compiler.clear();
							submission_detail = result_array[result_array.size() - 16 * current_page - detail.value().y];
							results_mode = submission_detail.submission_ID;
							detail_table.push_back_row({ U"Submission Time",submission_detail.submission_time.format() });
							detail_table.push_back_row({ U"Contest",submission_detail.contest_name });
							detail_table.push_back_row({ U"Task",submission_detail.task_name });
							detail_table.push_back_row({ U"Lang",submission_detail.lang });
							detail_table.push_back_row({ U"Status",submission_detail.result });
							detail_table.push_back_row({ U"Runtime",ToString(submission_detail.runtime) });
							if (submission_detail.lang == U"C++")detail_code = TextAreaEditState{ TextReader{U"data/code/" + submission_detail.submission_ID + U"/main.cpp"}.readAll() };
							else detail_code = TextAreaEditState{ TextReader{U"data/code/" + submission_detail.submission_ID + U"/main.py"}.readAll() };
							if (FileSystem::IsFile(U"data/code/" + submission_detail.submission_ID + U"/compiler_out.txt")) {
								detail_compiler = TextAreaEditState{ TextReader{U"data/code/" + submission_detail.submission_ID + U"/compiler_out.txt" }.readAll() };
							}
							Array<testcase_result>temp;
							JSON result = JSON::Load(U"data/code/" + results_mode + U"/result.json");
							testcase_table.push_back_row({ U"Case Name",U"Status",U"Runtime" });
							for (auto i : result[U"testcases"]) {
								testcase_table.push_back_row({ i.key,i.value[U"result"].getString(),i.value[U"runtime"].getString().substr(0,std::min(6,(int)i.value[U"runtime"].getString().size())) });
							}
							for (int i = 1; i < testcase_table.rows(); i++) {
								if (i % 2 == 1) {
									for (int j = 0; j < testcase_table.columns(); j++) {
										testcase_table.setBackgroundColor(i, j, Palette::Whitesmoke);
									}
								}
								String temp_str = testcase_table.getItem(i, 1).text;
								if (temp_str.includes(U"AC")) {
									testcase_table.setBackgroundColor(i, 1, Palette::Limegreen);
								}
								else {
									testcase_table.setBackgroundColor(i, 1, Palette::Goldenrod);
								}
							}
						}
					}
				}
				const auto detail = submit_table.cellIndex({ 280,40 }, Cursor::Pos());
				if (detail) {
					if (detail.value().x == 6 && detail.value().y > 0) {
						Cursor::RequestStyle(CursorStyle::Hand);
					}
				}
			}
			else {
				detail_table.clear();
				detail_table.push_back_row({ U"Submission Time",submission_detail.submission_time.format() });
				detail_table.push_back_row({ U"Contest",submission_detail.contest_name });
				detail_table.push_back_row({ U"Task",submission_detail.task_name });
				detail_table.push_back_row({ U"Lang",submission_detail.lang });
				detail_table.push_back_row({ U"Status",submission_detail.result });
				detail_table.push_back_row({ U"Runtime",ToString(submission_detail.runtime) });
				String temp = detail_table.getItem(4, 1).text;
				if (temp.includes(U"AC")) {
					detail_table.setBackgroundColor(4, 1, Palette::Limegreen);
				}
				else if (temp.includes(U"WJ")) {
					detail_table.setBackgroundColor(4, 1, Palette::Gray);
				}
				else {
					detail_table.setBackgroundColor(4, 1, Palette::Goldenrod);
				}
				scroll_detail -= Mouse::Wheel() * 80;
				if (scroll_detail > 0)scroll_detail = 0;
				if (std::min(-36 * ((int)testcase_table.rows() - 18), 0) > scroll_detail)scroll_detail = std::min(-36 * ((int)testcase_table.rows() - 18), 0);
				testcase_table.draw({ 800,scroll_detail + 30 });
				detail_table.draw({ 280,70 });
				nfont(U"Submission #" + submission_detail.submission_ID).draw(30, Arg::leftCenter(280, 50), ColorF{ 0.0 });
				nfont(U"Code").draw(30, Arg::bottomLeft(280, 330), ColorF{ 0.0 });
				nfont(U"Compiler Output").draw(30, Arg::bottomLeft(280, 573), ColorF{ 0.0 });
				if (SimpleGUI::Button(U"All Submissions", { 565,30 })) {
					results_mode = U"ALL";
				}
				if (SimpleGUI::Button(U"Copy", { 665,290 })) {
					Clipboard::SetText(detail_code.text);
				}
				if (SimpleGUI::Button(U"Copy", { 665,535 })) {
					Clipboard::SetText(detail_compiler.text);
				}
				SimpleGUI::TextArea(detail_code, { 280,330 }, { 470,200 }, 100000);
				SimpleGUI::TextArea(detail_compiler, { 280,575 }, { 470,130 }, 100000);
			}
		}
	}
	stop = true;
}
