#pragma once

#include <queue>
#include <vector>
#include <limits>
#include <cstdlib>
#include <utility>
#include <iostream>
#include <algorithm>
#include <functional>

namespace vptree {

class VPLevelPartition {
public:
    VPLevelPartition(double radius, unsigned int start, unsigned int end) {
        // For each partition, the vantage point is the first point within the partition (pointed by indexStart)

        _radius = radius;
        _indexStart = start;
        _indexEnd = end;
    }

    VPLevelPartition() {
        // Constructs an empty level

        _radius = 0;
        _indexStart = -1;
        _indexEnd = -1;
    }

    ~VPLevelPartition() {
        if( _left != nullptr )
            delete _left;

        if( _right != nullptr )
            delete _right;
    }

    bool isEmpty() { return _indexStart == -1 || _indexStart == -1; }
    unsigned int start() { return _indexStart; }
    unsigned int end() { return _indexEnd; }
    void setRadius(double radius) { _radius = radius; }
    double radius() { return _radius; }

    void setChild(VPLevelPartition* left, VPLevelPartition* right) {
        _left = left;
        _right = right;
    }

    VPLevelPartition* left() { return _left; }
    VPLevelPartition* right() { return _right; }

private:
    double _radius;

    // _indexStart and _indexEnd are index pointers to examples within the examples list, not index of coordinates
    // within the coordinate buffer.For instance, _indexEnd pointing to last element of a coordinate buffer of 9 entries
    // (3 examples of 3 dimensions each) would be pointing to 2, which is the index of the 3rd element.
    // If _indexStart == _indexEnd then the level contains only one element.
    unsigned int _indexStart; // points to the first of the example in which this level starts
    unsigned int _indexEnd;

    VPLevelPartition* _left = nullptr;
    VPLevelPartition* _right = nullptr;
};


template <typename T, double(*distance)(const T&, const T&)>
class VPTree {
public:

    struct VPTreeElement {

        VPTreeElement() = default;
        VPTreeElement(unsigned int index, const T& value) {
            originalIndex = index;
            val = value;
        }

        unsigned int originalIndex;
        T val;
    };

    struct VPTreeSearchResultElement {
        std::vector<unsigned int> indexes;
        std::vector<double> distances;
    };

    VPTree(const std::vector<T>& array) {

        _examples.reserve(array.size());
        _examples.resize(array.size());
        for(unsigned int i = 0; i < array.size(); ++i) {
            _examples[i] = VPTreeElement(i, array[i]);
        }

        build(_examples);
    }

    void search(const std::vector<T>& queries, unsigned int k, std::vector<VPTree::VPTreeSearchResultElement>& results) {

    }


protected:

    /*
     *  Builds a Vantage Point tree using each element of the given array as one coordinate buffer
     *  using the given metric distance.
     */
    void build(const std::vector<VPTreeElement>& array) {

        // Select vantage point
        std::vector<VPLevelPartition*> _toSplit;

        auto* root = new VPLevelPartition(-1, 0, _examples.size() - 1);
        _toSplit.push_back(root);
        _rootPartition = root;

        while(!_toSplit.empty()) {

            VPLevelPartition* current = _toSplit.back();
            _toSplit.pop_back();

            unsigned int start =  current->start();
            unsigned int end = current->end();

            unsigned vpIndex = selectVantagePoint(start, end);

            // put vantage point as the first element within the examples list
            std::swap(_examples[vpIndex], _examples[start]);

            unsigned int median = (end + start) / 2;

            // partition in order to keep all elements smaller than median in the left and larger in the right
            std::nth_element(_examples.begin() + start + 1, _examples.begin() + median, _examples.begin() + end, VPDistanceComparator(_examples[start]));

            // distance from vantage point (which is at start index) and the median element
            double medianDistance = distance(_examples[start].val, _examples[median].val);
            current->setRadius(medianDistance);

            // Schedule to build next levels
            //
            // Left is every one within the median distance radius
            auto* left = new VPLevelPartition(-1, start + 1, median);
            _toSplit.push_back(left);

            auto* right = new VPLevelPartition(-1, median + 1, end);
            _toSplit.push_back(right);

            current->setChild(left, right);
        }
    }

    // Internal temporary struct to organize K closest elements in a priorty queue
    struct VPTreeSearchElement {
        VPTreeSearchElement(int index, double dist) :
            index(index), dist(dist) {}
        int index;
        double dist;
        bool operator<(const VPTreeSearchElement& v) const {
            return dist < v.dist;
        }
    };

    void search(VPLevelPartition* partition, const T& val, unsigned int k) {

        std::priority_queue<VPTreeSearchElement> knnQueue;
        double tau = std::numeric_limits<double>::max();


        std::vector<VPLevelPartition*> toSearch = {partition};


        while(!toSearch.empty()) {

            auto* current = toSearch.back();
            toSearch.pop_back();

            if(current->left() == nullptr || current->right() == nullptr) {

                // perform exaustive search since there is no more subdivision
            }

            double dist = distance(val, _examples[current->start()].val);


            if(dist < partition.radius()) {
                // we need to seach inside the vantage point partition
                toSearch.push_back(partition->left());
            }

        }
    }

    unsigned int selectVantagePoint(unsigned int fromIndex, unsigned int toIndex) {

        // for now, simple random point selection as basic strategy: TODO: better vantage point selection
        // considering length of active region border (as in Yianilos (1993) paper)
        //
        assert( fromIndex >= 0 && fromIndex < _examples.size() && toIndex >= 0 && toIndex < _examples.size() && fromIndex <= toIndex && "fromIndex and toIndex must be in a valid range" );

        unsigned int range = (toIndex-fromIndex) + 1;
        return fromIndex + (rand() % range);
    }

    /*
     * A vantage point distance comparator. Will check which from two points are closer to the reference vantage point.
     * This is used to find the median distance from vantage point in order to split the VPLevelPartition into two sets.
     */
    struct VPDistanceComparator {

        const T& item;
        VPDistanceComparator( const VPTreeElement& item ) : item(item.val) {}
        bool operator()(const VPTreeElement& a, const VPTreeElement& b) {
            return distance( item, a.val ) < distance( item, b.val );
        }

    };

protected:

    std::vector<VPTreeElement> _examples;
    VPLevelPartition* _rootPartition = nullptr;
};

} // namespace vptree
