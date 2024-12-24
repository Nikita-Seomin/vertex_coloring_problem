#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <algorithm>
#include <unordered_set>
#include <time.h>
using namespace std;
#include <unordered_map>
#include <ctime>
#include <chrono>

using namespace std;

class ColoringProblem
{
public:
    void ReadGraphFile(const string &filename)
    {
        ifstream fin(filename);
        string line;
        int vertices = 0, edges = 0;

        while (getline(fin, line))
        {
            if (line[0] == 'c')
                continue; // Comments

            stringstream line_input(line);
            char command;

            if (line[0] == 'p')
            {
                string type;
                line_input >> command >> type >> vertices >> edges;
                neighbour_sets.resize(vertices);
                colors.resize(vertices + 1, 0);
            }
            else if (line[0] == 'e') // Edges
            {
                int start, finish;
                line_input >> command >> start >> finish;
                start--;  // Shift to zero index
                finish--; // Shift to zero index
                neighbour_sets[start].insert(finish);
                neighbour_sets[finish].insert(start);
            }
        }
    }

    void GreedyGraphColoring()
    {
        int n = neighbour_sets.size();
        colors.assign(n, 0); // Обнуляем цвета
        maxcolor = 0;

        // Применяем жадную раскраску для всех вершин
        for (int vertex = 0; vertex < n; ++vertex)
        {
            unordered_set<int> used_colors;

            // Собираем все цвета соседей
            for (int neighbor : neighbour_sets[vertex])
            {
                if (colors[neighbor] != 0)
                {
                    used_colors.insert(colors[neighbor]);
                }
            }

            // Назначаем наименьший доступный цвет
            int color = 1;
            while (used_colors.find(color) != used_colors.end())
            {
                color++;
            }
            colors[vertex] = color;
            maxcolor = max(maxcolor, color);
        }
    }

    void TabuSearchGraphColoring(int max_iterations, int tenure)
    {
        int n = neighbour_sets.size();

        // Инициализация решения: используем жадную раскраску
        GreedyGraphColoring();
        vector<int> best_colors = colors;
        vector<int> current_colors = colors;
        int best_color_count = maxcolor;

        // Табу-список, отслеживание запрещенных ходов
        vector<vector<int>> tabu_list(n, vector<int>(n, 0));

        // Вспомогательная функция, проверяющая количество конфликтов
        auto calculate_conflicts = [&]()
        {
            int conflicts = 0;
            for (int i = 0; i < n; ++i)
            {
                for (int neighbor : neighbour_sets[i])
                {
                    if (current_colors[i] == current_colors[neighbor])
                    {
                        ++conflicts;
                    }
                }
            }
            return conflicts / 2; // Каждое ребро считается дважды
        };

        // Стартовые значения
        int current_conflicts = calculate_conflicts();
        int iteration = 0;

        while (iteration < max_iterations && current_conflicts > 0)
        {
            ++iteration;
            int best_delta = INT_MAX;
            int best_vertex = -1;
            int best_new_color = -1;

            // Поиск наилучшего улучшения (или наименьшего вреда)
            for (int vertex = 0; vertex < n; ++vertex)
            {
                int old_color = current_colors[vertex];
                unordered_set<int> neighbor_colors;

                // Собираем цвета соседей
                for (int neighbor : neighbour_sets[vertex])
                {
                    neighbor_colors.insert(current_colors[neighbor]);
                }

                // Пробуем все возможные цвета
                for (int color = 1; color <= best_color_count + 1; ++color)
                {
                    if (color == old_color)
                        continue;

                    // Если цвет уже запрещён, проверяем табу-условие
                    if (tabu_list[vertex][color] > iteration)
                        continue;

                    // Подсчитаем, сколько конфликтов изменится
                    current_colors[vertex] = color;
                    int new_conflicts = calculate_conflicts();

                    int delta = new_conflicts - current_conflicts;
                    if (delta < best_delta || (delta == best_delta && rand() % 2 == 0))
                    {
                        best_delta = delta;
                        best_vertex = vertex;
                        best_new_color = color;
                    }
                }

                // Возвращаем старый цвет
                current_colors[vertex] = old_color;
            }

            // Применяем наилучшее найденное улучшение
            if (best_vertex != -1 && best_new_color != -1)
            {
                int previous_color = current_colors[best_vertex];
                current_colors[best_vertex] = best_new_color;

                // Обновляем табу-список
                tabu_list[best_vertex][previous_color] = iteration + tenure;

                current_conflicts += best_delta;

                // Проверяем, улучшилось ли лучшее известное решение
                if (current_conflicts < calculate_conflicts())
                {
                    best_colors = current_colors;
                    best_color_count = *max_element(best_colors.begin(), best_colors.end());
                }
            }
        }

        // Устанавливаем лучшее найденное решение
        colors = best_colors;
        maxcolor = best_color_count;
    }


    void PardalosGraphColoring()
    {
        int n = neighbour_sets.size();
        vector<int> vertex_order(n);
        for (int i = 0; i < n; i++)
        {
            vertex_order[i] = i;
        }

        // Sort vertices by degree in descending order
        sort(vertex_order.begin(), vertex_order.end(),
             [&](int a, int b)
             {
                 return neighbour_sets[a].size() > neighbour_sets[b].size();
             });

        for (int vertex : vertex_order)
        {
            unordered_set<int> used_colors;

            // Collect colors of neighbors
            for (int neighbor : neighbour_sets[vertex])
            {
                if (colors[neighbor] != 0)
                {
                    used_colors.insert(colors[neighbor]);
                }
            }

            // Assign the smallest available color
            int color = 1;
            while (used_colors.find(color) != used_colors.end())
            {
                color++;
            }
            colors[vertex] = color;
            maxcolor = max(maxcolor, color);
        }
    }

    bool Check()
    {
        for (size_t i = 0; i < neighbour_sets.size(); ++i)
        {
            if (colors[i] == 0)
            {
                cerr << "Vertex " << i + 1 << " is not colored\n";
                return false;
            }
            for (int neighbor : neighbour_sets[i])
            {
                if (colors[neighbor] == colors[i])
                {
                    cerr << "Neighbour vertices " << i + 1 << ", " << neighbor + 1 << " have the same color\n";
                    return false;
                }
            }
        }
        return true;
    }

    int GetNumberOfColors()
    {
        return maxcolor;
    }

    const vector<int> &GetColors()
    {
        return colors;
    }

    void PrintColors()
    {
        unordered_map<int, vector<int>> color_classes;

        // Group vertices by their color
        for (size_t i = 0; i < colors.size(); ++i)
        {
            if (colors[i] != 0)
            {
                color_classes[colors[i]].push_back(i + 1);
            }
        }

        cout << maxcolor << endl;
        for (const auto &pair : color_classes)
        {
            cout << "{";
            for (size_t i = 0; i < pair.second.size(); ++i){
                cout << pair.second[i];
                if (i != pair.second.size() - 1)
                    cout << ",";
            }
            cout << "}";
            if (pair.first != maxcolor)
                cout << ", ";
        }
        cout << endl;
    }

private:
    vector<int> colors;
    int maxcolor = 0;
    vector<unordered_set<int>> neighbour_sets;
};

int main()
{
    vector<string> files = { "myciel3.col", "myciel7.col", "latin_square_10.col", "school1.col", "school1_nsh.col",
        "mulsol.i.1.col", "inithx.i.1.col", "anna.col", "huck.col", "jean.col", "miles1000.col", "miles1500.col",
        "fpsol2.i.1.col", "le450_5a.col", "le450_15b.col", "le450_25a.col", "games120.col",
        "queen11_11.col", "queen5_5.col" };
    ofstream fout("color.csv");
    fout << "Instance; Colors; Time (sec)\n";
    cout << "Instance; Colors; Time (sec)\n";
    for (const string &file : files)
    {
        ColoringProblem problem;
        problem.ReadGraphFile(file);

        auto start = chrono::steady_clock::now(); // Запуск таймера
        problem.GreedyGraphColoring();
        auto end = chrono::steady_clock::now(); // Остановка таймера

        // Продолжительность в секундах
        double elapsed_time = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1e6;
        if (!problem.Check())
        {
            cerr << "Warning: Incorrect coloring detected!" << endl;
        }

        cout << file << " " << problem.GetNumberOfColors() << " "
             << elapsed_time << endl;
        problem.PrintColors();
    }
    fout.close();
    return 0;
}