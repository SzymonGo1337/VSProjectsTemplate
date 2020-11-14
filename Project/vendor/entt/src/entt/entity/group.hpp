#ifndef ENTT_ENTITY_GROUP_HPP
#define ENTT_ENTITY_GROUP_HPP


#include <tuple>
#include <utility>
#include <type_traits>
#include "../config/config.h"
#include "../core/type_traits.hpp"
#include "entity.hpp"
#include "fwd.hpp"
#include "pool.hpp"
#include "sparse_set.hpp"
#include "utility.hpp"


namespace entt {


/**
 * @brief Group.
 *
 * Primary template isn't defined on purpose. All the specializations give a
 * compile-time error, but for a few reasonable cases.
 */
template<typename...>
class basic_group;


/**
 * @brief Non-owning group.
 *
 * A non-owning group returns all entities and only the entities that have at
 * least the given components. Moreover, it's guaranteed that the entity list
 * is tightly packed in memory for fast iterations.
 *
 * @b Important
 *
 * Iterators aren't invalidated if:
 *
 * * New instances of the given components are created and assigned to entities.
 * * The entity currently pointed is modified (as an example, if one of the
 *   given components is removed from the entity to which the iterator points).
 * * The entity currently pointed is destroyed.
 *
 * In all other cases, modifying the pools iterated by the group in any way
 * invalidates all the iterators and using them results in undefined behavior.
 *
 * @note
 * Groups share references to the underlying data structures of the registry
 * that generated them. Therefore any change to the entities and to the
 * components made by means of the registry are immediately reflected by all the
 * groups.<br/>
 * Moreover, sorting a non-owning group affects all the instances of the same
 * group (it means that users don't have to call `sort` on each instance to sort
 * all of them because they _share_ entities and components).
 *
 * @warning
 * Lifetime of a group must not overcome that of the registry that generated it.
 * In any other case, attempting to use a group results in undefined behavior.
 *
 * @tparam Entity A valid entity type (see entt_traits for more details).
 * @tparam Exclude Types of components used to filter the group.
 * @tparam Get Type of components observed by the group.
 */
template<typename Entity, typename... Exclude, typename... Get>
class basic_group<Entity, exclude_t<Exclude...>, get_t<Get...>> final {
    /*! @brief A registry is allowed to create groups. */
    friend class basic_registry<Entity>;

    template<typename Component>
    using pool_type = pool_t<Entity, Component>;

    class iterable_group {
        friend class basic_group<Entity, exclude_t<Exclude...>, get_t<Get...>>;

        template<typename, typename>
        class iterable_group_iterator;

        template<typename It, typename... Type>
        class iterable_group_iterator<It, type_list<Type...>> {
            friend class iterable_group;

            template<typename... Args>
            iterable_group_iterator(It from, const std::tuple<pool_type<Get> *...> &args) ENTT_NOEXCEPT
                : it{from},
                  pools{std::get<pool_type<Type> *>(args)...}
            {}

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = decltype(std::tuple_cat(std::tuple<Entity>{}, std::declval<basic_group>().get({})));
            using pointer = void;
            using reference = value_type;
            using iterator_category = std::input_iterator_tag;

            iterable_group_iterator & operator++() ENTT_NOEXCEPT {
                return ++it, *this;
            }

            iterable_group_iterator operator++(int) ENTT_NOEXCEPT {
                iterable_group_iterator orig = *this;
                return ++(*this), orig;
            }

            [[nodiscard]] reference operator*() const ENTT_NOEXCEPT {
                const auto entt = *it;
                return std::tuple_cat(std::make_tuple(entt), std::forward_as_tuple(std::get<pool_type<Type> *>(pools)->get(entt)...));
            }

            [[nodiscard]] bool operator==(const iterable_group_iterator &other) const ENTT_NOEXCEPT {
                return other.it == it;
            }

            [[nodiscard]] bool operator!=(const iterable_group_iterator &other) const ENTT_NOEXCEPT {
                return !(*this == other);
            }

        private:
            It it;
            const std::tuple<pool_type<Type> *...> pools;
        };

        iterable_group(basic_sparse_set<Entity> &ref, const std::tuple<pool_type<Get> *...> &cpools)
            : handler{&ref},
              pools{cpools}
        {}

    public:
        using iterator = iterable_group_iterator<
            typename basic_sparse_set<Entity>::iterator,
            type_list_cat_t<std::conditional_t<is_eto_eligible_v<Get>, type_list<>, type_list<Get>>...>
        >;
        using reverse_iterator = iterable_group_iterator<
            typename basic_sparse_set<Entity>::reverse_iterator,
            type_list_cat_t<std::conditional_t<is_eto_eligible_v<Get>, type_list<>, type_list<Get>>...>
        >;

        [[nodiscard]] iterator begin() const ENTT_NOEXCEPT {
            return { handler->begin(), pools };
        }

        [[nodiscard]] iterator end() const ENTT_NOEXCEPT {
            return { handler->end(), pools };
        }

        [[nodiscard]] reverse_iterator rbegin() const ENTT_NOEXCEPT {
            return { handler->rbegin(), pools };
        }

        [[nodiscard]] reverse_iterator rend() const ENTT_NOEXCEPT {
            return { handler->rend(), pools };
        }

    private:
        basic_sparse_set<Entity> *handler;
        const std::tuple<pool_type<Get> *...> pools;
    };

    basic_group(basic_sparse_set<Entity> &ref, pool_type<Get> &... gpool) ENTT_NOEXCEPT
        : handler{&ref},
          pools{&gpool...}
    {}

    template<typename Func, typename... Weak>
    void traverse(Func func, type_list<Weak...>) const {
        for(const auto entt: *handler) {
            if constexpr(std::is_invocable_v<Func, entity_type, decltype(get<Weak>({}))...>) {
                func(entt, std::get<pool_type<Weak> *>(pools)->get(entt)...);
            } else {
                func(std::get<pool_type<Weak> *>(pools)->get(entt)...);
            }
        }
    }

public:
    /*! @brief Underlying entity identifier. */
    using entity_type = Entity;
    /*! @brief Unsigned integer type. */
    using size_type = std::size_t;
    /*! @brief Random access iterator type. */
    using iterator = typename basic_sparse_set<Entity>::iterator;
    /*! @brief Reversed iterator type. */
    using reverse_iterator = typename basic_sparse_set<Entity>::reverse_iterator;

    /**
     * @brief Returns the number of existing components of the given type.
     * @tparam Component Type of component of which to return the size.
     * @return Number of existing components of the given type.
     */
    template<typename Component>
    [[nodiscard]] size_type size() const ENTT_NOEXCEPT {
        return std::get<pool_type<Component> *>(pools)->size();
    }

    /**
     * @brief Returns the number of entities that have the given components.
     * @return Number of entities that have the given components.
     */
    [[nodiscard]] size_type size() const ENTT_NOEXCEPT {
        return handler->size();
    }

    /**
     * @brief Returns the number of elements that a group has currently
     * allocated space for.
     * @return Capacity of the group.
     */
    [[nodiscard]] size_type capacity() const ENTT_NOEXCEPT {
        return handler->capacity();
    }

    /*! @brief Requests the removal of unused capacity. */
    void shrink_to_fit() {
        handler->shrink_to_fit();
    }

    /**
     * @brief Checks whether a group or some pools are empty.
     * @tparam Component Types of components in which one is interested.
     * @return True if the group or the pools are empty, false otherwise.
     */
    template<typename... Component>
    [[nodiscard]] bool empty() const ENTT_NOEXCEPT {
        if constexpr(sizeof...(Component) == 0) {
            return handler->empty();
        } else {
            return (std::get<pool_type<Component> *>(pools)->empty() && ...);
        }
    }

    /**
     * @brief Direct access to the list of components of a given pool.
     *
     * The returned pointer is such that range
     * `[raw<Component>(), raw<Component>() + size<Component>())` is always a
     * valid range, even if the container is empty.
     *
     * @note
     * Components are in the reverse order as returned by the `begin`/`end`
     * iterators.
     *
     * @tparam Component Type of component in which one is interested.
     * @return A pointer to the array of components.
     */
    template<typename Component>
    [[nodiscard]] Component * raw() const ENTT_NOEXCEPT {
        return std::get<pool_type<Component> *>(pools)->raw();
    }

    /**
     * @brief Direct access to the list of entities of a given pool.
     *
     * The returned pointer is such that range
     * `[data<Component>(), data<Component>() + size<Component>())` is always a
     * valid range, even if the container is empty.
     *
     * @note
     * Entities are in the reverse order as returned by the `begin`/`end`
     * iterators.
     *
     * @tparam Component Type of component in which one is interested.
     * @return A pointer to the array of entities.
     */
    template<typename Component>
    [[nodiscard]] const entity_type * data() const ENTT_NOEXCEPT {
        return std::get<pool_type<Component> *>(pools)->data();
    }

    /**
     * @brief Direct access to the list of entities.
     *
     * The returned pointer is such that range `[data(), data() + size())` is
     * always a valid range, even if the container is empty.
     *
     * @note
     * Entities are in the reverse order as returned by the `begin`/`end`
     * iterators.
     *
     * @return A pointer to the array of entities.
     */
    [[nodiscard]] const entity_type * data() const ENTT_NOEXCEPT {
        return handler->data();
    }

    /**
     * @brief Returns an iterator to the first entity of the group.
     *
     * The returned iterator points to the first entity of the group. If the
     * group is empty, the returned iterator will be equal to `end()`.
     *
     * @return An iterator to the first entity of the group.
     */
    [[nodiscard]] iterator begin() const ENTT_NOEXCEPT {
        return handler->begin();
    }

    /**
     * @brief Returns an iterator that is past the last entity of the group.
     *
     * The returned iterator points to the entity following the last entity of
     * the group. Attempting to dereference the returned iterator results in
     * undefined behavior.
     *
     * @return An iterator to the entity following the last entity of the
     * group.
     */
    [[nodiscard]] iterator end() const ENTT_NOEXCEPT {
        return handler->end();
    }

    /**
     * @brief Returns an iterator to the first entity of the reversed group.
     *
     * The returned iterator points to the first entity of the reversed group.
     * If the group is empty, the returned iterator will be equal to `rend()`.
     *
     * @return An iterator to the first entity of the reversed group.
     */
    [[nodiscard]] reverse_iterator rbegin() const ENTT_NOEXCEPT {
        return handler->rbegin();
    }

    /**
     * @brief Returns an iterator that is past the last entity of the reversed
     * group.
     *
     * The returned iterator points to the entity following the last entity of
     * the reversed group. Attempting to dereference the returned iterator
     * results in undefined behavior.
     *
     * @return An iterator to the entity following the last entity of the
     * reversed group.
     */
    [[nodiscard]] reverse_iterator rend() const ENTT_NOEXCEPT {
        return handler->rend();
    }

    /**
     * @brief Returns the first entity of the group, if any.
     * @return The first entity of the group if one exists, the null entity
     * otherwise.
     */
    [[nodiscard]] entity_type front() const {
        const auto it = begin();
        return it != end() ? *it : null;
    }

    /**
     * @brief Returns the last entity of the group, if any.
     * @return The last entity of the group if one exists, the null entity
     * otherwise.
     */
    [[nodiscard]] entity_type back() const {
        const auto it = rbegin();
        return it != rend() ? *it : null;
    }

    /**
     * @brief Finds an entity.
     * @param entt A valid entity identifier.
     * @return An iterator to the given entity if it's found, past the end
     * iterator otherwise.
     */
    [[nodiscard]] iterator find(const entity_type entt) const {
        const auto it = handler->find(entt);
        return it != end() && *it == entt ? it : end();
    }

    /**
     * @brief Returns the identifier that occupies the given position.
     * @param pos Position of the element to return.
     * @return The identifier that occupies the given position.
     */
    [[nodiscard]] entity_type operator[](const size_type pos) const {
        return begin()[pos];
    }

    /**
     * @brief Checks if a group contains an entity.
     * @param entt A valid entity identifier.
     * @return True if the group contains the given entity, false otherwise.
     */
    [[nodiscard]] bool contains(const entity_type entt) const {
        return handler->contains(entt);
    }

    /**
     * @brief Returns the components assigned to the given entity.
     *
     * Prefer this function instead of `registry::get` during iterations. It has
     * far better performance than its counterpart.
     *
     * @warning
     * Attempting to use an invalid component type results in a compilation
     * error. Attempting to use an entity that doesn't belong to the group
     * results in undefined behavior.<br/>
     * An assertion will abort the execution at runtime in debug mode if the
     * group doesn't contain the given entity.
     *
     * @tparam Component Types of components to get.
     * @param entt A valid entity identifier.
     * @return The components assigned to the entity.
     */
    template<typename... Component>
    [[nodiscard]] decltype(auto) get(const entity_type entt) const {
        ENTT_ASSERT(contains(entt));

        if constexpr(sizeof...(Component) == 0) {
            return std::tuple_cat([entt](auto *cpool) {
                if constexpr(is_eto_eligible_v<typename std::remove_reference_t<decltype(*cpool)>::value_type>) {
                    return std::tuple{};
                } else {
                    return std::forward_as_tuple(cpool->get(entt));
                }
            }(std::get<pool_type<Get> *>(pools))...);
        } else if constexpr(sizeof...(Component) == 1) {
            return (std::get<pool_type<Component> *>(pools)->get(entt), ...);
        } else {
            return std::forward_as_tuple(get<Component>(entt)...);
        }
    }

    /**
     * @brief Iterates entities and components and applies the given function
     * object to them.
     *
     * The function object is invoked for each entity. It is provided with the
     * entity itself and a set of references to non-empty components. The
     * _constness_ of the components is as requested.<br/>
     * The signature of the function must be equivalent to one of the following
     * forms:
     *
     * @code{.cpp}
     * void(const entity_type, Type &...);
     * void(Type &...);
     * @endcode
     *
     * @note
     * Empty types aren't explicitly instantiated and therefore they are never
     * returned during iterations.
     *
     * @tparam Func Type of the function object to invoke.
     * @param func A valid function object.
     */
    template<typename Func>
    void each(Func func) const {
        traverse(std::move(func), (std::conditional_t<is_eto_eligible_v<Get>, type_list<>, type_list<Get>>{} + ...));
    }

    /**
     * @brief Returns an iterable object to use to _visit_ the group.
     *
     * The iterable object returns tuples that contain the current entity and a
     * set of references to its non-empty components. The _constness_ of the
     * components is as requested.
     *
     * @note
     * Empty types aren't explicitly instantiated and therefore they are never
     * returned during iterations.
     *
     * @return An iterable object to use to _visit_ the group.
     */
    [[nodiscard]] iterable_group each() const ENTT_NOEXCEPT {
        return iterable_group{*handler, pools};
    }

    /**
     * @brief Sort a group according to the given comparison function.
     *
     * Sort the group so that iterating it with a couple of iterators returns
     * entities and components in the expected order. See `begin` and `end` for
     * more details.
     *
     * The comparison function object must return `true` if the first element
     * is _less_ than the second one, `false` otherwise. The signature of the
     * comparison function should be equivalent to one of the following:
     *
     * @code{.cpp}
     * bool(std::tuple<Component &...>, std::tuple<Component &...>);
     * bool(const Component &..., const Component &...);
     * bool(const Entity, const Entity);
     * @endcode
     *
     * Where `Component` are such that they are iterated by the group.<br/>
     * Moreover, the comparison function object shall induce a
     * _strict weak ordering_ on the values.
     *
     * The sort function oject must offer a member function template
     * `operator()` that accepts three arguments:
     *
     * * An iterator to the first element of the range to sort.
     * * An iterator past the last element of the range to sort.
     * * A comparison function to use to compare the elements.
     *
     * @tparam Component Optional types of components to compare.
     * @tparam Compare Type of comparison function object.
     * @tparam Sort Type of sort function object.
     * @tparam Args Types of arguments to forward to the sort function object.
     * @param compare A valid comparison function object.
     * @param algo A valid sort function object.
     * @param args Arguments to forward to the sort function object, if any.
     */
    template<typename... Component, typename Compare, typename Sort = std_sort, typename... Args>
    void sort(Compare compare, Sort algo = Sort{}, Args &&... args) {
        if constexpr(sizeof...(Component) == 0) {
            static_assert(std::is_invocable_v<Compare, const entity_type, const entity_type>, "Invalid comparison function");
            handler->sort(handler->begin(), handler->end(), std::move(compare), std::move(algo), std::forward<Args>(args)...);
        }  else if constexpr(sizeof...(Component) == 1) {
            handler->sort(handler->begin(), handler->end(), [this, compare = std::move(compare)](const entity_type lhs, const entity_type rhs) {
                return compare((std::get<pool_type<Component> *>(pools)->get(lhs), ...), (std::get<pool_type<Component> *>(pools)->get(rhs), ...));
            }, std::move(algo), std::forward<Args>(args)...);
        } else {
            handler->sort(handler->begin(), handler->end(), [this, compare = std::move(compare)](const entity_type lhs, const entity_type rhs) {
                return compare(std::tuple<decltype(get<Component>({}))...>{std::get<pool_type<Component> *>(pools)->get(lhs)...}, std::tuple<decltype(get<Component>({}))...>{std::get<pool_type<Component> *>(pools)->get(rhs)...});
            }, std::move(algo), std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Sort the shared pool of entities according to the given component.
     *
     * Non-owning groups of the same type share with the registry a pool of
     * entities with its own order that doesn't depend on the order of any pool
     * of components. Users can order the underlying data structure so that it
     * respects the order of the pool of the given component.
     *
     * @note
     * The shared pool of entities and thus its order is affected by the changes
     * to each and every pool that it tracks. Therefore changes to those pools
     * can quickly ruin the order imposed to the pool of entities shared between
     * the non-owning groups.
     *
     * @tparam Component Type of component to use to impose the order.
     */
    template<typename Component>
    void sort() const {
        handler->respect(*std::get<pool_type<Component> *>(pools));
    }

private:
    basic_sparse_set<entity_type> *handler;
    const std::tuple<pool_type<Get> *...> pools;
};


/**
 * @brief Owning group.
 *
 * Owning groups return all entities and only the entities that have at least
 * the given components. Moreover:
 *
 * * It's guaranteed that the entity list is tightly packed in memory for fast
 *   iterations.
 * * It's guaranteed that the lists of owned components are tightly packed in
 *   memory for even faster iterations and to allow direct access.
 * * They stay true to the order of the owned components and all instances have
 *   the same order in memory.
 *
 * The more types of components are owned by a group, the faster it is to
 * iterate them.
 *
 * @b Important
 *
 * Iterators aren't invalidated if:
 *
 * * New instances of the given components are created and assigned to entities.
 * * The entity currently pointed is modified (as an example, if one of the
 *   given components is removed from the entity to which the iterator points).
 * * The entity currently pointed is destroyed.
 *
 * In all other cases, modifying the pools iterated by the group in any way
 * invalidates all the iterators and using them results in undefined behavior.
 *
 * @note
 * Groups share references to the underlying data structures of the registry
 * that generated them. Therefore any change to the entities and to the
 * components made by means of the registry are immediately reflected by all the
 * groups.
 * Moreover, sorting an owning group affects all the instance of the same group
 * (it means that users don't have to call `sort` on each instance to sort all
 * of them because they share the underlying data structure).
 *
 * @warning
 * Lifetime of a group must not overcome that of the registry that generated it.
 * In any other case, attempting to use a group results in undefined behavior.
 *
 * @tparam Entity A valid entity type (see entt_traits for more details).
 * @tparam Exclude Types of components used to filter the group.
 * @tparam Get Types of components observed by the group.
 * @tparam Owned Types of components owned by the group.
 */
template<typename Entity, typename... Exclude, typename... Get, typename... Owned>
class basic_group<Entity, exclude_t<Exclude...>, get_t<Get...>, Owned...> final {
    /*! @brief A registry is allowed to create groups. */
    friend class basic_registry<Entity>;

    template<typename Component>
    using pool_type = pool_t<Entity, Component>;

    class iterable_group {
        friend class basic_group<Entity, exclude_t<Exclude...>, get_t<Get...>, Owned...>;

        template<typename, typename, typename>
        class iterable_group_iterator;

        template<typename It, typename... OIt, typename... Type>
        class iterable_group_iterator<It, type_list<OIt...>, type_list<Type...>> {
            friend class iterable_group;

            template<typename... Other>
            iterable_group_iterator(It from, const std::tuple<Other...> &other, const std::tuple<pool_type<Get> *...> &cpools) ENTT_NOEXCEPT
                : it{from},
                  owned{std::get<OIt>(other)...},
                  get{std::get<pool_type<Type> *>(cpools)...}
            {}

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = decltype(std::tuple_cat(std::tuple<Entity>{}, std::declval<basic_group>().get({})));
            using pointer = void;
            using reference = value_type;
            using iterator_category = std::input_iterator_tag;

            iterable_group_iterator & operator++() ENTT_NOEXCEPT {
                return ++it, (++std::get<OIt>(owned), ...), *this;
            }

            iterable_group_iterator operator++(int) ENTT_NOEXCEPT {
                iterable_group_iterator orig = *this;
                return ++(*this), orig;
            }

            [[nodiscard]] reference operator*() const ENTT_NOEXCEPT {
                return std::tuple_cat(
                    std::make_tuple(*it),
                    std::forward_as_tuple(*std::get<OIt>(owned)...),
                    std::forward_as_tuple(std::get<pool_type<Type> *>(get)->get(*it)...)
                );
            }

            [[nodiscard]] bool operator==(const iterable_group_iterator &other) const ENTT_NOEXCEPT {
                return other.it == it;
            }

            [[nodiscard]] bool operator!=(const iterable_group_iterator &other) const ENTT_NOEXCEPT {
                return !(*this == other);
            }

        private:
            It it;
            std::tuple<OIt...> owned;
            std::tuple<pool_type<Type> *...> get;
        };

        iterable_group(std::tuple<pool_type<Owned> *..., pool_type<Get> *...> cpools, const std::size_t &extent)
            : pools{cpools},
              length{&extent}
        {}

    public:
        using iterator = iterable_group_iterator<
            typename basic_sparse_set<Entity>::iterator,
            type_list_cat_t<std::conditional_t<is_eto_eligible_v<Owned>, type_list<>, type_list<decltype(std::declval<pool_type<Owned>>().end())>>...>,
            type_list_cat_t<std::conditional_t<is_eto_eligible_v<Get>, type_list<>, type_list<Get>>...>
        >;
        using reverse_iterator = iterable_group_iterator<
            typename basic_sparse_set<Entity>::reverse_iterator,
            type_list_cat_t<std::conditional_t<is_eto_eligible_v<Owned>, type_list<>, type_list<decltype(std::declval<pool_type<Owned>>().rbegin())>>...>,
            type_list_cat_t<std::conditional_t<is_eto_eligible_v<Get>, type_list<>, type_list<Get>>...>
        >;

        [[nodiscard]] iterator begin() const ENTT_NOEXCEPT {
            return {
                std::get<0>(pools)->basic_sparse_set<Entity>::end() - *length,
                std::make_tuple((std::get<pool_type<Owned> *>(pools)->end() - *length)...),
                std::make_tuple(std::get<pool_type<Get> *>(pools)...)
            };
        }

        [[nodiscard]] iterator end() const ENTT_NOEXCEPT {
            return {
                std::get<0>(pools)->basic_sparse_set<Entity>::end(),
                std::make_tuple((std::get<pool_type<Owned> *>(pools)->end())...),
                std::make_tuple(std::get<pool_type<Get> *>(pools)...)
            };
        }

        [[nodiscard]] reverse_iterator rbegin() const ENTT_NOEXCEPT {
            return {
                std::get<0>(pools)->basic_sparse_set<Entity>::rbegin(),
                std::make_tuple((std::get<pool_type<Owned> *>(pools)->rbegin())...),
                std::make_tuple(std::get<pool_type<Get> *>(pools)...)
            };
        }

        [[nodiscard]] reverse_iterator rend() const ENTT_NOEXCEPT {
            return {
                std::get<0>(pools)->basic_sparse_set<Entity>::rbegin() + *length,
                std::make_tuple((std::get<pool_type<Owned> *>(pools)->rbegin() + *length)...),
                std::make_tuple(std::get<pool_type<Get> *>(pools)...)
            };
        }

    private:
        const std::tuple<pool_type<Owned> *..., pool_type<Get> *...> pools;
        const std::size_t *length;
    };

    basic_group(const std::size_t &extent, pool_type<Owned> &... opool, pool_type<Get> &... gpool) ENTT_NOEXCEPT
        : pools{&opool..., &gpool...},
          length{&extent}
    {}

    template<typename Func, typename... Strong, typename... Weak>
    void traverse(Func func, type_list<Strong...>, type_list<Weak...>) const {
        [[maybe_unused]] auto it = std::make_tuple((std::get<pool_type<Strong> *>(pools)->end() - *length)...);
        [[maybe_unused]] auto data = std::get<0>(pools)->basic_sparse_set<entity_type>::end() - *length;

        for(auto next = *length; next; --next) {
            const auto entt = *(data++);

            if constexpr(std::is_invocable_v<Func, entity_type, decltype(get<Strong>({}))..., decltype(get<Weak>({}))...>) {
                func(entt, *(std::get<decltype(std::get<pool_type<Strong> *>(pools)->end())>(it)++)..., std::get<pool_type<Weak> *>(pools)->get(entt)...);
            } else {
                func(*(std::get<decltype(std::get<pool_type<Strong> *>(pools)->end())>(it)++)..., std::get<pool_type<Weak> *>(pools)->get(entt)...);
            }
        }
    }

public:
    /*! @brief Underlying entity identifier. */
    using entity_type = Entity;
    /*! @brief Unsigned integer type. */
    using size_type = std::size_t;
    /*! @brief Random access iterator type. */
    using iterator = typename basic_sparse_set<Entity>::iterator;
    /*! @brief Reversed iterator type. */
    using reverse_iterator = typename basic_sparse_set<Entity>::reverse_iterator;

    /**
     * @brief Returns the number of existing components of the given type.
     * @tparam Component Type of component of which to return the size.
     * @return Number of existing components of the given type.
     */
    template<typename Component>
    [[nodiscard]] size_type size() const ENTT_NOEXCEPT {
        return std::get<pool_type<Component> *>(pools)->size();
    }

    /**
     * @brief Returns the number of entities that have the given components.
     * @return Number of entities that have the given components.
     */
    [[nodiscard]] size_type size() const ENTT_NOEXCEPT {
        return *length;
    }

    /**
     * @brief Checks whether a group or some pools are empty.
     * @tparam Component Types of components in which one is interested.
     * @return True if the group or the pools are empty, false otherwise.
     */
    template<typename... Component>
    [[nodiscard]] bool empty() const ENTT_NOEXCEPT {
        if constexpr(sizeof...(Component) == 0) {
            return !*length;
        } else {
            return (std::get<pool_type<Component> *>(pools)->empty() && ...);
        }
    }

    /**
     * @brief Direct access to the list of components of a given pool.
     *
     * The returned pointer is such that range
     * `[raw<Component>(), raw<Component>() + size<Component>())` is always a
     * valid range, even if the container is empty.<br/>
     * Moreover, in case the group owns the given component, the range
     * `[raw<Component>(), raw<Component>() + size())` is such that it contains
     * the instances that are part of the group itself.
     *
     * @note
     * Components are in the reverse order as returned by the `begin`/`end`
     * iterators.
     *
     * @tparam Component Type of component in which one is interested.
     * @return A pointer to the array of components.
     */
    template<typename Component>
    [[nodiscard]] Component * raw() const ENTT_NOEXCEPT {
        return std::get<pool_type<Component> *>(pools)->raw();
    }

    /**
     * @brief Direct access to the list of entities of a given pool.
     *
     * The returned pointer is such that range
     * `[data<Component>(), data<Component>() + size<Component>())` is always a
     * valid range, even if the container is empty.<br/>
     * Moreover, in case the group owns the given component, the range
     * `[data<Component>(), data<Component>() + size())` is such that it
     * contains the entities that are part of the group itself.
     *
     * @note
     * Entities are in the reverse order as returned by the `begin`/`end`
     * iterators.
     *
     * @tparam Component Type of component in which one is interested.
     * @return A pointer to the array of entities.
     */
    template<typename Component>
    [[nodiscard]] const entity_type * data() const ENTT_NOEXCEPT {
        return std::get<pool_type<Component> *>(pools)->data();
    }

    /**
     * @brief Direct access to the list of entities.
     *
     * The returned pointer is such that range `[data(), data() + size())` is
     * always a valid range, even if the container is empty.
     *
     * @note
     * Entities are in the reverse order as returned by the `begin`/`end`
     * iterators.
     *
     * @return A pointer to the array of entities.
     */
    [[nodiscard]] const entity_type * data() const ENTT_NOEXCEPT {
        return std::get<0>(pools)->data();
    }

    /**
     * @brief Returns an iterator to the first entity of the group.
     *
     * The returned iterator points to the first entity of the group. If the
     * group is empty, the returned iterator will be equal to `end()`.
     *
     * @return An iterator to the first entity of the group.
     */
    [[nodiscard]] iterator begin() const ENTT_NOEXCEPT {
        return std::get<0>(pools)->basic_sparse_set<entity_type>::end() - *length;
    }

    /**
     * @brief Returns an iterator that is past the last entity of the group.
     *
     * The returned iterator points to the entity following the last entity of
     * the group. Attempting to dereference the returned iterator results in
     * undefined behavior.
     *
     * @return An iterator to the entity following the last entity of the
     * group.
     */
    [[nodiscard]] iterator end() const ENTT_NOEXCEPT {
        return std::get<0>(pools)->basic_sparse_set<entity_type>::end();
    }

    /**
     * @brief Returns an iterator to the first entity of the reversed group.
     *
     * The returned iterator points to the first entity of the reversed group.
     * If the group is empty, the returned iterator will be equal to `rend()`.
     *
     * @return An iterator to the first entity of the reversed group.
     */
    [[nodiscard]] reverse_iterator rbegin() const ENTT_NOEXCEPT {
        return std::get<0>(pools)->basic_sparse_set<entity_type>::rbegin();
    }

    /**
     * @brief Returns an iterator that is past the last entity of the reversed
     * group.
     *
     * The returned iterator points to the entity following the last entity of
     * the reversed group. Attempting to dereference the returned iterator
     * results in undefined behavior.
     *
     * @return An iterator to the entity following the last entity of the
     * reversed group.
     */
    [[nodiscard]] reverse_iterator rend() const ENTT_NOEXCEPT {
        return std::get<0>(pools)->basic_sparse_set<entity_type>::rbegin() + *length;
    }

    /**
     * @brief Returns the first entity of the group, if any.
     * @return The first entity of the group if one exists, the null entity
     * otherwise.
     */
    [[nodiscard]] entity_type front() const {
        const auto it = begin();
        return it != end() ? *it : null;
    }

    /**
     * @brief Returns the last entity of the group, if any.
     * @return The last entity of the group if one exists, the null entity
     * otherwise.
     */
    [[nodiscard]] entity_type back() const {
        const auto it = rbegin();
        return it != rend() ? *it : null;
    }

    /**
     * @brief Finds an entity.
     * @param entt A valid entity identifier.
     * @return An iterator to the given entity if it's found, past the end
     * iterator otherwise.
     */
    [[nodiscard]] iterator find(const entity_type entt) const {
        const auto it = std::get<0>(pools)->find(entt);
        return it != end() && it >= begin() && *it == entt ? it : end();
    }

    /**
     * @brief Returns the identifier that occupies the given position.
     * @param pos Position of the element to return.
     * @return The identifier that occupies the given position.
     */
    [[nodiscard]] entity_type operator[](const size_type pos) const {
        return begin()[pos];
    }

    /**
     * @brief Checks if a group contains an entity.
     * @param entt A valid entity identifier.
     * @return True if the group contains the given entity, false otherwise.
     */
    [[nodiscard]] bool contains(const entity_type entt) const {
        return std::get<0>(pools)->contains(entt) && (std::get<0>(pools)->index(entt) < (*length));
    }

    /**
     * @brief Returns the components assigned to the given entity.
     *
     * Prefer this function instead of `registry::get` during iterations. It has
     * far better performance than its counterpart.
     *
     * @warning
     * Attempting to use an invalid component type results in a compilation
     * error. Attempting to use an entity that doesn't belong to the group
     * results in undefined behavior.<br/>
     * An assertion will abort the execution at runtime in debug mode if the
     * group doesn't contain the given entity.
     *
     * @tparam Component Types of components to get.
     * @param entt A valid entity identifier.
     * @return The components assigned to the entity.
     */
    template<typename... Component>
    [[nodiscard]] decltype(auto) get(const entity_type entt) const {
        ENTT_ASSERT(contains(entt));

        if constexpr(sizeof...(Component) == 0) {
            auto filter = [entt, index = std::get<0>(pools)->index(entt)](auto *cpool) {
                using value_type = typename std::remove_reference_t<decltype(*cpool)>::value_type;

                if constexpr(is_eto_eligible_v<value_type>) {
                    return std::tuple{};
                } else if constexpr((std::is_same_v<value_type, std::remove_const_t<Owned>> || ...)) {
                    return std::forward_as_tuple(cpool->raw()[index]);
                } else {
                    return std::forward_as_tuple(cpool->get(entt));
                }
            };

            return std::tuple_cat(filter(std::get<pool_type<Owned> *>(pools))..., filter(std::get<pool_type<Get> *>(pools))...);
        } else if constexpr(sizeof...(Component) == 1) {
            return (std::get<pool_type<Component> *>(pools)->get(entt), ...);
        } else {
            return std::forward_as_tuple(get<Component>(entt)...);
        }
    }

    /**
     * @brief Iterates entities and components and applies the given function
     * object to them.
     *
     * The function object is invoked for each entity. It is provided with the
     * entity itself and a set of references to non-empty components. The
     * _constness_ of the components is as requested.<br/>
     * The signature of the function must be equivalent to one of the following
     * forms:
     *
     * @code{.cpp}
     * void(const entity_type, Type &...);
     * void(Type &...);
     * @endcode
     *
     * @note
     * Empty types aren't explicitly instantiated and therefore they are never
     * returned during iterations.
     *
     * @tparam Func Type of the function object to invoke.
     * @param func A valid function object.
     */
    template<typename Func>
    void each(Func func) const {
        traverse(std::move(func),
            (std::conditional_t<is_eto_eligible_v<Owned>, type_list<>, type_list<Owned>>{} + ...),
            (type_list<>{} + ... + std::conditional_t<is_eto_eligible_v<Get>, type_list<>, type_list<Get>>{}));
    }

    /**
     * @brief Returns an iterable object to use to _visit_ the group.
     *
     * The iterable object returns tuples that contain the current entity and a
     * set of references to its non-empty components. The _constness_ of the
     * components is as requested.
     *
     * @note
     * Empty types aren't explicitly instantiated and therefore they are never
     * returned during iterations.
     *
     * @return An iterable object to use to _visit_ the group.
     */
    [[nodiscard]] iterable_group each() const ENTT_NOEXCEPT {
        return iterable_group{pools, *length};
    }

    /**
     * @brief Sort a group according to the given comparison function.
     *
     * Sort the group so that iterating it with a couple of iterators returns
     * entities and components in the expected order. See `begin` and `end` for
     * more details.
     *
     * The comparison function object must return `true` if the first element
     * is _less_ than the second one, `false` otherwise. The signature of the
     * comparison function should be equivalent to one of the following:
     *
     * @code{.cpp}
     * bool(std::tuple<Component &...>, std::tuple<Component &...>);
     * bool(const Component &, const Component &);
     * bool(const Entity, const Entity);
     * @endcode
     *
     * Where `Component` are either owned types or not but still such that they
     * are iterated by the group.<br/>
     * Moreover, the comparison function object shall induce a
     * _strict weak ordering_ on the values.
     *
     * The sort function oject must offer a member function template
     * `operator()` that accepts three arguments:
     *
     * * An iterator to the first element of the range to sort.
     * * An iterator past the last element of the range to sort.
     * * A comparison function to use to compare the elements.
     *
     * @tparam Component Optional types of components to compare.
     * @tparam Compare Type of comparison function object.
     * @tparam Sort Type of sort function object.
     * @tparam Args Types of arguments to forward to the sort function object.
     * @param compare A valid comparison function object.
     * @param algo A valid sort function object.
     * @param args Arguments to forward to the sort function object, if any.
     */
    template<typename... Component, typename Compare, typename Sort = std_sort, typename... Args>
    void sort(Compare compare, Sort algo = Sort{}, Args &&... args) {
        auto *cpool = std::get<0>(pools);

        if constexpr(sizeof...(Component) == 0) {
            static_assert(std::is_invocable_v<Compare, const entity_type, const entity_type>, "Invalid comparison function");
            cpool->sort(cpool->end()-*length, cpool->end(), std::move(compare), std::move(algo), std::forward<Args>(args)...);
        } else if constexpr(sizeof...(Component) == 1) {
            cpool->sort(cpool->end()-*length, cpool->end(), [this, compare = std::move(compare)](const entity_type lhs, const entity_type rhs) {
                return compare((std::get<pool_type<Component> *>(pools)->get(lhs), ...), (std::get<pool_type<Component> *>(pools)->get(rhs), ...));
            }, std::move(algo), std::forward<Args>(args)...);
        } else {
            cpool->sort(cpool->end()-*length, cpool->end(), [this, compare = std::move(compare)](const entity_type lhs, const entity_type rhs) {
                return compare(std::tuple<decltype(get<Component>({}))...>{std::get<pool_type<Component> *>(pools)->get(lhs)...}, std::tuple<decltype(get<Component>({}))...>{std::get<pool_type<Component> *>(pools)->get(rhs)...});
            }, std::move(algo), std::forward<Args>(args)...);
        }

        [this](auto *head, auto *... other) {
            for(auto next = *length; next; --next) {
                const auto pos = next - 1;
                [[maybe_unused]] const auto entt = head->data()[pos];
                (other->swap(other->data()[pos], entt), ...);
            }
        }(std::get<pool_type<Owned> *>(pools)...);
    }

private:
    const std::tuple<pool_type<Owned> *..., pool_type<Get> *...> pools;
    const size_type *length;
};


}


#endif