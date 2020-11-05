#include <iostream>
#include <string>
#include <functional>
#include <type_traits>
#include <utility>

//============================================================
//	\class	functionRef
//
//	\author	KeyC0de
//	\date	2020/11/05 3:17
//
//	\brief	a non-owning wrapper for an arbitrary callable object
//=============================================================
template <typename Callable>
class functionRef;

template <typename TReturn, typename... TArgs>
class functionRef<TReturn(TArgs...)>
{
	using CallbackType = TReturn(void*, TArgs...);
	using CallbackPointerType = std::add_pointer_t<CallbackType>;

	void* m_addr;
	CallbackPointerType m_pfn;
public:
	functionRef()
		: m_addr{ nullptr },
		m_pfn{ nullptr }
	{}
	functionRef(std::nullptr_t)
		: m_addr{ nullptr },
		m_pfn{ nullptr }
	{}

	template <typename F,
		typename = std::enable_if_t<std::is_convertible_v<std::result_of_t<F(TArgs...)>, TReturn>>,
		typename = std::enable_if_t<std::is_invocable_r_v<TReturn, F, TArgs...>>,
		typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, functionRef>>
	>
		constexpr functionRef(F&& callable)
		: m_addr{ (void*)std::addressof(callable) }
	{
		//static_assert(!std::is_member_function_pointer_v<F>, "This class doesn't accept member pointers yet..");
		if (std::addressof(callable) != nullptr)
		{
			m_pfn = [](void* po, TArgs... args) -> TReturn
			{
				return (*reinterpret_cast<std::add_pointer_t<F>>(po))(std::forward<TArgs>(args)...);
			};
		}
		else
		{
			m_addr = nullptr;
			m_pfn = nullptr;
		}
	}

	/// \brief this is the destructor for functionRef not the underlying object
	~functionRef() noexcept
	{
		reset();
	}

	functionRef(const functionRef& rhs)
	{
		void* temp_addr{ rhs.m_addr };
		CallbackPointerType temp_pfn{ rhs.m_pfn };
		std::swap(m_addr, temp_addr);
		std::swap(m_pfn, temp_pfn);
	}
	functionRef& operator=(const functionRef& rhs)
	{
		if (this != &rhs)
		{
			void* temp_addr{ rhs.m_addr };
			CallbackPointerType temp_pfn{ rhs.m_pfn };
			std::swap(m_addr, temp_addr);
			std::swap(m_pfn, temp_pfn);
		}
		return *this;
	}

	functionRef(functionRef&& rhs) noexcept
		: m_addr{ std::move(rhs.m_addr) },
		m_pfn{ std::move(rhs.m_pfn) }
	{}
	functionRef &operator=(functionRef&& rhs) noexcept
	{
		if (this != &rhs)
		{
			std::swap(m_addr, rhs.m_addr);
			std::swap(m_pfn, rhs.m_pfn);
		}
		return *this;
	}

	void swap(functionRef& rhs) noexcept
	{
		std::swap(m_addr, rhs.m_addr);
		std::swap(m_pfn, rhs.m_pfn);
	}

	template <typename T, typename FType, 
		typename = std::enable_if_t<std::is_member_function_pointer_v<FType>>>
	void setTarget(T& obj, FType f)
	{
		if (std::addressof(obj) != nullptr)
		{
			m_addr = reinterpret_cast<void*>(std::addressof(obj));
			m_pfn = reinterpret_cast<TReturn(*)(void*, TArgs...)>((void*&)f);
		}
		else
		{
			m_addr = nullptr;
			m_pfn = nullptr;
		}
	}

	// Delete r-values. Not interested in temporary objects here
	template <typename T, typename FType>
	void setTarget(T&&) = delete;
	
	bool reset() noexcept
	{
		if (m_addr || m_pfn)
		{
			m_addr = nullptr;
			m_pfn = nullptr;
			return true;
		}
		return false;
	}

	inline TReturn operator()(TArgs... args) const
	{
		return m_pfn(m_addr, std::forward<TArgs>(args)...);
	}

	operator bool() const noexcept
	{
		return !(m_addr == nullptr || m_pfn == nullptr);
	}

	[[nodiscard]] inline constexpr bool operator==(const functionRef& rhs) const noexcept { return this->m_addr == rhs.m_addr; }
	[[nodiscard]] inline constexpr bool operator!=(const functionRef& rhs) const noexcept { return this->m_addr != rhs.m_addr; }
	[[nodiscard]] inline constexpr bool operator==(const functionRef* rhs) const noexcept { return this->m_addr == rhs->m_addr; }
	[[nodiscard]] inline constexpr bool operator!=(const functionRef* rhs) const noexcept { return this->m_addr != rhs->m_addr; }
};
//////////////////////////////////////////////////////////////////////////////////////////

