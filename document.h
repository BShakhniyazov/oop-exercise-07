#pragma once

#include <fstream>
#include <functional>
#include <deque>
#include "factory.h"

//document
template<class T>
class Document
{
public:
    Document() {}
    //new document
    void New()
    {
        figures.clear();
        undoTrace.clear();
        redoTrace.clear();
    }
    //save document
    void Save(std::string fname)
    {
        std::ofstream out(fname);
        if (!out.good()) throw std::logic_error("cannot open file " + fname); //throw if cannot open file
        //save all figures
        out << figures.size() << std::endl; //number of figures
        for (auto &ptr: figures)
        {
            auto fig = *ptr;
            out << fig.v.size() << " "; //number of vertices
            //center & vertex
            auto cntr = center(fig);
            out << cntr.first << " " << cntr.second << " " << fig.v[0].first << " " << fig.v[0].second << std::endl;
        }
    }
    //load document
    void Load(std::string fname)
    {
        std::ifstream in(fname);
        if (!in.good()) throw std::logic_error("cannot open file " + fname); //throw if cannot open file

        New(); //clear everything
        int sz; in >> sz; //number of figures
        while (sz--)
        {
            int nvertex; //number of vertices
            T ox, oy, ax, ay; //center % vertex
            in >> nvertex >> ox >> oy >> ax >> ay;
            figures.push_back(Factory<T>::Generate(nvertex, ox, oy, ax, ay));
        }
    }
    //add figure
    void Add(int i, int n, T ox, T oy, T ax, T ay)
    {
        if (i < 0 || i > figures.size())
            throw std::out_of_range(std::to_string(i) + " out of bounds [0, " + std::to_string(figures.size()) + "]");

        ///clear redo data
        redoTrace.clear();
        //save data for undo
        undoTrace.push_back(Action(at_added, i, n, ox, oy, ax, ay));
        //push figure
        figures.insert(figures.begin() + i, Factory<T>::Generate(n, ox, oy, ax, ay));
    }
    //remove figure
    void Remove(int i)
    {
        if (i < 0 || i >= figures.size())
            throw std::out_of_range(std::to_string(i) + " out of bounds [0, " + std::to_string(figures.size()) + ")");

        ///clear redo data
        redoTrace.clear();
        //save data for undo
        auto fig = *(figures[i]);
        auto c = center(fig);
        undoTrace.push_back(Action(at_removed, i, fig.v.size(),
            c.first, c.second, fig.v[0].first, fig.v[0].second));
        //remove figure
        figures.erase(figures.begin() + i);
    }
    //call lambda for figure
    template<class U>
    U Call(int i, std::function<U (Ngon<T>)> lambda)
    {
        if (i < 0 || i >= figures.size())
            throw std::out_of_range(std::to_string(i) + " out of bounds [0, " + std::to_string(figures.size()) + ")");

        return lambda(*(figures[i]));
    }
    //call lambda for all figures
    void ForEach(std::function<void (Ngon<T>)> lambda)
    {
        for (auto &ptr : figures) lambda(*ptr);
    }
    //undo action
    void Undo()
    {
        if (undoTrace.size() == 0) throw std::logic_error("nothing to undo"); //do nothing
        Action action = undoTrace.back(); //get action
        undoTrace.pop_back();
        //add action to redo trace
        redoTrace.push_back(action);
        //perform
        switch (action.type)
        {
        case at_added: //remove if added
            figures.erase(figures.begin() + action.index);
            break;
        case at_removed: //add if removed
            figures.insert(figures.begin() + action.index,
                Factory<T>::Generate(action.n, action.ox, action.oy, action.ax, action.ay));
            break;
        }
    }
    //redo action
    void Redo()
    {
        if (redoTrace.size() == 0) throw std::logic_error("nothing to redo"); //do nothing
        Action action = redoTrace.back(); //get action
        redoTrace.pop_back();
        //add action to undo trace
        undoTrace.push_back(action);
        //perform
        switch (action.type)
        {
        case at_added: //add if added
            figures.insert(figures.begin() + action.index,
                Factory<T>::Generate(action.n, action.ox, action.oy, action.ax, action.ay));
            break;
        case at_removed: //remove if removed
            figures.erase(figures.begin() + action.index);
            break;
        }
    }

private:
    std::vector<std::shared_ptr<Ngon<T>>> figures;

    enum AType { at_added, at_removed };
    struct Action
    {
        AType type;
        int index, n;
        T ox, oy, ax, ay;
        Action(AType t, int i, int _n, T _ox, T _oy, T _ax, T _ay) :
            type(t), index(i), n(_n), ox(_ox), oy(_oy), ax(_ax), ay(_ay) { }
    };
    std::deque<Action> undoTrace, redoTrace;
};
