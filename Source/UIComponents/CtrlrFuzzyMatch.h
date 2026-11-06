/*
  ==============================================================================

    CtrlrFuzzyMatch.h
    Created: 4 Nov 2026 8:23:46pm
    Author:  zan64

  ==============================================================================
*/

#pragma once
namespace CtrlrFuzz
{
    inline bool isSubsequence(const juce::String& q, const juce::String& s)
    {
        int qi = 0;
        for (int i = 0; i < s.length() && qi < q.length(); ++i)
            if (s[i] == q[qi])
                ++qi;
        return qi == q.length();
    }

    // Returns 0 for "no match", otherwise higher = better.
    inline double scoreMatch(const juce::String& query, const juce::String& name)
    {
        const juce::String ql = query.toLowerCase();
        const juce::String nl = name.toLowerCase();
        double score = 0.0;

        if (nl.startsWith(ql))             score = 100.0;
        else if (nl.contains(ql))          score = 90.0;
        else if (isSubsequence(ql, nl))    score = 70.0;
        else if (ql.length() >= 4)
        {
            const double f = rapidfuzz::fuzz::partial_ratio(ql.toStdString(), nl.toStdString());
            if (f >= 85.0)
                score = f * 0.6;
        }

        if (score > 0.0)
            score -= nl.length() * 0.01;   // tie-break: shorter names first

        return score;
    }
}