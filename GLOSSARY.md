# Polygon Symmetry Finder

Finds every mirror axis of a convex polygon read from a list of 2D points. Rotational symmetry is out of scope.

## Shape

**Polygon**:
A convex polygon given by its vertices in boundary order, closed implicitly (the last vertex connects back to the first), with duplicate and collinear vertices already removed.
_Avoid_: Hull, contour, point set

**Edge**:
The segment between two consecutive vertices, including the closing segment from the last vertex to the first.
_Avoid_: Side, prev vector

**Original index**:
The position a vertex had in the points given to the polygon, before cleanup removed any of them; reported results refer to vertices by it.
_Avoid_: Input index

**Collinear vertex**:
A vertex lying within the collinearity threshold (r) of the segment between its two original neighbours (a vertex beyond either neighbour is a spike, not collinear); it is not part of the polygon's shape and is dropped before matching. r must not exceed τ, so cleanup moves the outline by at most τ; the executable uses τ itself.
_Avoid_: Straight vertex, redundant point

## Symmetry

**Tolerance (τ)**:
The largest distance allowed between a reflected vertex and its partner vertex.
_Avoid_: Precision, epsilon, uncertainty

**Mirror axis**:
A line such that every vertex, reflected across it, lands within τ of the vertex at the mirrored position in boundary order (τ-symmetry).
_Avoid_: Symmetry (alone), palindrome, reflection line, ε-uncertainty

**Candidate axis**:
A mirror axis proposed by symbol matching that has not yet been checked against the polygon itself.
_Avoid_: Possible axis, palindrome

**Axis anchor point**:
Where a mirror axis meets the polygon's boundary: a vertex, or the midpoint of an edge. Every axis of a convex polygon has exactly two, and a reported axis is given by them.
_Avoid_: Chord point, endpoint, axis point

**Verified axis**:
A candidate axis proven to be a mirror axis.
_Avoid_: Confirmed axis, exact axis

**Ambiguous axis**:
A candidate axis that could be neither proven nor disproven, because its measured deviation lies within the margin for estimating the axis line.
_Avoid_: Probable axis, uncertain axis

**Rejected candidate**:
A candidate axis proven not to be a mirror axis; it is never reported.
_Avoid_: False axis

**Candidate group**:
All candidate axes of one polygon taken together; they are always evenly spaced, like the axes of a regular polygon, and are verified as one unit.
_Avoid_: Candidate set, axis list

## Matching

**Feature**:
A scalar measured on one edge or one vertex that a reflection leaves unchanged, such as an edge length or the angle at a vertex.
_Avoid_: Invariant, characteristic

**Compatibility class**:
A group of feature values merged so that any two values that could be equal within tolerance always share a group.
_Avoid_: Bucket, cluster, equivalence class

**Symbol**:
The compatibility classes of one edge or one vertex, compared by exact equality.
_Avoid_: Letter, token

**Symbol string**:
The cyclic sequence of symbols that alternates edge symbols and vertex symbols around the polygon.
_Avoid_: Invariant string, feature string
