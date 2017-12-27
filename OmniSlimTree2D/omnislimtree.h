#ifndef OMNISLIMTREE_H
#define OMNISLIMTREE_H

#include "slimnodeleaf.h"
#include <QDebug>

template <class T>
class SlimTree
{
    SlimNode<T> * m_pRoot;
    stDist (*m_fDist)(T, T);
    int m_Capacity;
public:
    stDist (*m_fDistAux)(T, T);
    SlimTree(int NodeCapacity = 0, stDist (*f)(T,T) = 0, stDist (*f_aux)(T,T) = 0)
    {
        m_pRoot = nullptr;
        m_fDist = f;
        m_fDistAux = f_aux;
        m_Capacity = NodeCapacity;
    }
    vector<T> m_Foci;
    virtual ~SlimTree(){}
    void AddElement(T d)
    {
        if (!m_pRoot)
        {
            SlimNodeLeaf<T> * r = new SlimNodeLeaf<T>(m_Capacity);
            r->AddElement(d, m_fDist);
            m_pRoot = r;
            return;
        }
        if (m_pRoot->GetTipo() == SlimNode<T>::LEAF)
        {
            SlimNodeLeaf<T> * p = static_cast< SlimNodeLeaf<T> * >(m_pRoot);
            if (p->AddElement(d, m_fDist))
            {
                m_pRoot = p->m_pPadre;
            }
        }
        else
        {
            SlimNodeIdx<T> * p = static_cast< SlimNodeIdx<T> * >(m_pRoot);
            SlimNode<T> *q = m_pRoot;
            while (q->GetTipo() == SlimNode<T>::INDEX)
            {
                int MinNElements_index, MinDist_index;
                int NElementsMin = m_Capacity + 1;
                stDist MinDist = -1;
                bool Qualified = false;
                for (int i = 0; i < q->m_Elements.size(); i++)
                {
                    stDist Distance = m_fDist(d, q->m_Elements[i]->m_Dato);
                    if (MinDist == -1 || Distance < MinDist)
                    {
                        MinDist = Distance;
                        MinDist_index = i;
                    }
                    if (Distance <= static_cast< typename SlimNodeIdx<T>::IdxElement * >(q->m_Elements[i])->m_Radio &&
                        static_cast< typename SlimNodeIdx<T>::IdxElement * >(q->m_Elements[i])->m_NElementsSubT < NElementsMin)
                    {
                        Qualified = true;
                        MinNElements_index = i;
                        NElementsMin = static_cast< typename SlimNodeIdx<T>::IdxElement * >(q->m_Elements[i])->m_NElementsSubT;
                    }
                }
                if (!Qualified) q = static_cast< typename SlimNodeIdx<T>::IdxElement * >(q->m_Elements[MinDist_index])->m_pSubtree;
                else q = static_cast< typename SlimNodeIdx<T>::IdxElement * >(q->m_Elements[MinNElements_index])->m_pSubtree;
            }
            if (static_cast< SlimNodeLeaf<T>* >(q)->AddElement(d, m_fDist))
            {
                m_pRoot = p->m_pPadre;
            }
        }
    }
    bool DeleteElement(T d)
    {
        if (!m_pRoot) return false;
        SlimNodeLeaf<T> * ContainerNode = nullptr;
        int PosInNode;
        queue< SlimNode<T> * > SearchQ;
        SearchQ.push(m_pRoot);
        bool found = false;
        while (!SearchQ.empty())
        {
            SlimNode<T> * N = SearchQ.front();
            for (int i=0; i < N->m_Elements.size(); i++)
            {
                if (N->GetTipo() == SlimNode<T>::LEAF)
                {
                    if (m_fDist(d, N->m_Elements[i]->m_Dato) <= 0)
                    {
                        ContainerNode = static_cast< SlimNodeLeaf<T> * >(N);
                        PosInNode = i;
                        found = true;
                        break;
                    }
                }
                else
                {
                    if (m_fDist(d, N->m_Elements[i]->m_Dato) <= static_cast< typename SlimNodeIdx<T>::IdxElement * >(N->m_Elements[i])->m_Radio)
                        SearchQ.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(N->m_Elements[i])->m_pSubtree);
                }
            }
            if (found) break;
            SearchQ.pop();
        }
        if (found)
        {
            ContainerNode->DeleteElement(PosInNode, m_fDist);
            return true;
        }
        return false;
    }
    void SlimDownLeaves(int Limit = 20)
    {
        vector< SlimNodeLeaf<T> * > LeafQ;
        queue< SlimNode<T> * > SearchQ;
        SearchQ.push(m_pRoot);
        while (!SearchQ.empty())
        {
            SlimNode<T> * N = SearchQ.front();
            if (N->GetTipo() == SlimNode<T>::LEAF)
                LeafQ.push_back(static_cast< SlimNodeLeaf<T> * >(N));
            else
                for (int i=0; i < N->m_Elements.size(); i++)
                    SearchQ.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(N->m_Elements[i])->m_pSubtree);
            SearchQ.pop();
        }
        bool Change = true;
        int RoundCounter = 0;
        while (Change)
        {
            Change = false;
            RoundCounter++;
            for (int i = 0; i < LeafQ.size(); i++)
            {
                int FurthestElement = LeafQ[i]->GetFurthestElement();
                vector< pair< SlimNode<T> * , typename SlimNode<T>::BaseElement * > > Hermanos = LeafQ[i]->GetHermanos();
                for (int j = 0; j < Hermanos.size(); j++)
                {
                    stDist Distance = m_fDist(LeafQ[i]->m_Elements[FurthestElement]->m_Dato, Hermanos[j].second->m_Dato);
                    if (Distance <= static_cast< typename SlimNodeIdx<T>::IdxElement * >(Hermanos[j].second)->m_Radio &&
                        Hermanos[j].first->CheckIfFits())
                    {
                        T temp_data = LeafQ[i]->m_Elements[FurthestElement]->m_Dato;
                        LeafQ[i]->DeleteElement(FurthestElement, m_fDist);
                        if (LeafQ[i]->m_Elements.size() == 0) LeafQ.erase(LeafQ.begin()+i);
                        static_cast< SlimNodeLeaf<T> * >(Hermanos[j].first)->AddElement(temp_data, m_fDist);
                        Change = true;
                        break;
                    }
                }
            }
            if (RoundCounter > Limit) Change = false;
        }
    }
    void AddFocusDistance(T focus)
    {
        queue< SlimNode<T> * > Q;
        Q.push(m_pRoot);
        while (!Q.empty())
        {
            SlimNode<T> * p = Q.front();
            for (int i = 0; i < p->m_Elements.size(); i++)
            {
                stDist D = m_fDist(focus, p->m_Elements[i]->m_Dato);
                p->m_Elements[i]->m_dFoci.push_back(D);
                if (p->GetTipo() == SlimNode<T>::INDEX)
                    Q.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(p->m_Elements[i])->m_pSubtree);
            }
            Q.pop();
        }
    }
    T FindFarthest(T center)
    {
        T farthest;
        stDist dist2data = 0.0;
        queue< SlimNode<T> * > Q;
        Q.push(m_pRoot);
        while (!Q.empty())
        {
            SlimNode<T> * p = Q.front();
            for (int i = 0; i < p->m_Elements.size(); i++)
            {
                if (p->GetTipo() == SlimNode<T>::INDEX)
                    Q.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(p->m_Elements[i])->m_pSubtree);
                else
                {
                    stDist D = m_fDist(center, p->m_Elements[i]->m_Dato);
                    if (dist2data < D)
                    {
                        dist2data = D;
                        farthest = p->m_Elements[i]->m_Dato;
                    }
                }
            }
            Q.pop();
        }
        return farthest;
    }
    void FindFoci(int FociRequired = 2)
    {
        if (!m_pRoot) return;
        m_Foci.clear();
        T temp_data = m_pRoot->m_Elements[m_pRoot->GetRepPos()]->m_Dato;
        T focus1 = FindFarthest(temp_data);
        m_Foci.push_back(focus1);
        if (FociRequired <= 1)
        {
            AddFocusDistance(focus1);
            return;
        }
        T focus2;
        stDist dist2focus1 = 0.0;
        queue< SlimNode<T> * > Q;
        Q.push(m_pRoot);
        while (!Q.empty())
        {
            SlimNode<T> * p = Q.front();
            for (int i = 0; i < p->m_Elements.size(); i++)
            {
                stDist D = m_fDist(focus1, p->m_Elements[i]->m_Dato);
                p->m_Elements[i]->m_dFoci.push_back(D);
                if (dist2focus1 < D)
                {
                    dist2focus1 = D;
                    focus2 = p->m_Elements[i]->m_Dato;
                }
                if (p->GetTipo() == SlimNode<T>::INDEX)
                    Q.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(p->m_Elements[i])->m_pSubtree);
            }
            Q.pop();
        }
        m_Foci.push_back(focus2);
        if (FociRequired <= 2)
        {
            AddFocusDistance(focus2);
            return;
        }
        stDist focusEdge = m_fDist(focus1, focus2);
        int FociFound = 2;
        while (FociFound < FociRequired)
        {
            T focus;
            stDist minError = -1;
            Q.push(m_pRoot);
            while (!Q.empty())
            {
                SlimNode<T> * p = Q.front();
                for (int i = 0; i < p->m_Elements.size(); i++)
                {
                    stDist error = 0.0;
                    stDist D = m_fDist(p->m_Elements[i]->m_Dato, m_Foci[FociFound-1]);
                    p->m_Elements[i]->m_dFoci.push_back(D);
                    for (int k = 0; k < FociFound; k++)
                        error += abs(focusEdge - p->m_Elements[i]->m_dFoci[k]);
                    if (minError == -1 || error < minError)
                    {
                        bool isFocus = false;
                        for (int k = 0; k < FociFound; k++)
                        {
                            if (m_Foci[k] == p->m_Elements[i]->m_Dato)
                            {
                                isFocus = true;
                                break;
                            }
                        }
                        if (!isFocus)
                        {
                            minError = error;
                            focus = p->m_Elements[i]->m_Dato;
                        }
                    }
                    if (p->GetTipo() == SlimNode<T>::INDEX)
                        Q.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(p->m_Elements[i])->m_pSubtree);
                }
                Q.pop();
            }
            m_Foci.push_back(focus);
            FociFound++;
        }
        AddFocusDistance(m_Foci[FociRequired-1]);
    }
    vector<T> RangeQuery (T d, stDist Qrange, bool UseFoci = 1)
    {
        if (!m_Foci.empty() && UseFoci)
        {
            vector<T> Query;
            if (!m_pRoot) return Query;
            vector<T> inMbor;
            vector<stDist> dFociQuery;
            for (int i = 0; i < m_Foci.size(); i++)
                dFociQuery.push_back(m_fDist(d, m_Foci[i]));
            queue< SlimNode<T> * > SearchQ;
            SearchQ.push(m_pRoot);
            while (!SearchQ.empty())
            {
                SlimNode<T> * N = SearchQ.front();
                for (int i = 0; i < N->m_Elements.size(); i++)
                {
                    bool inIntersection = true;
                    if (N->GetTipo() == SlimNode<T>::LEAF)
                    {
                        for (int k = 0; k < m_Foci.size(); k++)
                        {
                            if (!(dFociQuery[k] - Qrange <= N->m_Elements[i]->m_dFoci[k] &&
                                  N->m_Elements[i]->m_dFoci[k] <= dFociQuery[k] + Qrange))
                            {
                                inIntersection = false;
                                break;
                            }
                        }
                        if (inIntersection) inMbor.push_back(N->m_Elements[i]->m_Dato);
                    }
                    else
                    {
                        for (int k = 0; k < m_Foci.size(); k++)
                        {
                            if (N->m_Elements[i]->m_dFoci[k] < dFociQuery[k] - Qrange)
                            {
                                if (N->m_Elements[i]->m_dFoci[k] + static_cast< typename SlimNodeIdx<T>::IdxElement * >(N->m_Elements[i])->m_Radio < dFociQuery[k] - Qrange)
                                {
                                    inIntersection = false;
                                    break;
                                }
                            }
                            else if (N->m_Elements[i]->m_dFoci[k] > dFociQuery[k] + Qrange)
                            {
                                if (N->m_Elements[i]->m_dFoci[k] - static_cast< typename SlimNodeIdx<T>::IdxElement * >(N->m_Elements[i])->m_Radio > dFociQuery[k] + Qrange)
                                {
                                    inIntersection = false;
                                    break;
                                }
                            }
                        }
                        if (inIntersection) SearchQ.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(N->m_Elements[i])->m_pSubtree);
                    }
                }
                SearchQ.pop();
            }
            for (int i = 0; i < inMbor.size(); i++)
            {
                if (m_fDist(d, inMbor[i]) <= Qrange)
                    Query.push_back(inMbor[i]);
            }
            return Query;
        }
        else
        {
            vector<T> Query;
            if (!m_pRoot) return Query;
            queue< SlimNode<T> * > SearchQ;
            SearchQ.push(m_pRoot);
            while (!SearchQ.empty())
            {
                SlimNode<T> * N = SearchQ.front();
                for (int i=0; i < N->m_Elements.size(); i++)
                {
                    if (N->GetTipo() == SlimNode<T>::LEAF)
                    {
                        if (m_fDist(d, N->m_Elements[i]->m_Dato) <= Qrange)
                            Query.push_back(N->m_Elements[i]->m_Dato);
                    }
                    else
                    {
                        if (m_fDist(d, N->m_Elements[i]->m_Dato) - Qrange <= static_cast< typename SlimNodeIdx<T>::IdxElement * >(N->m_Elements[i])->m_Radio)
                            SearchQ.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(N->m_Elements[i])->m_pSubtree);
                    }
                }
                SearchQ.pop();
            }
            return Query;
        }
    }
    pair< vector< pair<T, stDist> >, vector<T> > GetTreeData()
    {
        queue< SlimNode<T> * > Q;
        vector<T> Data;
        vector< pair<T, stDist> > Radios;
        Q.push(m_pRoot);
        while (!Q.empty())
        {
            SlimNode<T> * p = Q.front();
            for (int i = 0; i < p->m_Elements.size(); i++)
            {
                if (p->GetTipo() == SlimNode<T>::INDEX)
                {
                    Q.push(static_cast< typename SlimNodeIdx<T>::IdxElement * >(p->m_Elements[i])->m_pSubtree);
                    stDist R = static_cast< typename SlimNodeIdx<T>::IdxElement * >(p->m_Elements[i])->m_Radio;
                    if (R != 0.0)
                        Radios.push_back(pair< T, stDist >(p->m_Elements[i]->m_Dato, R));
                }
                else
                {
                    Data.push_back(p->m_Elements[i]->m_Dato);
                }
            }
            Q.pop();
        }
        return pair< vector< pair<T, stDist> >, vector<T> >(Radios, Data);
    }
};

#endif // OMNISLIMTREE_H
