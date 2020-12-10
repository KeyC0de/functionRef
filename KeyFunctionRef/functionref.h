#include <iostream>

//============================================================
//	\class	functionRef
//
//	\author	KeyC0de
//	\date	2019/11/05 3:17
//
//	\brief	a non-owning wrapper for an arbitrary callable object
//=============================================================
template <typename Callable>
class functionRef;

template <typename TReturn,
	typename... TArgs>
class functionRef<TReturn(TArgs...)>
{
	using CallbackType = TReturn( void*, TArgs... );
	using CallbackPointerType = std::add_pointer_t<CallbackType>;

	void* m_pAddr;
	CallbackPointerType m_pFn;
public:
	functionRef()
		:
		m_pAddr{ nullptr },
		m_pFn{ nullptr }
	{}
	functionRef( std::nullptr_t )
		:
		m_pAddr{ nullptr },
		m_pFn{ nullptr }
	{}

	//===================================================
	//	\function	functionRef<...>
	//	\brief  templated constructor
	//	\date	2019/11/20 19:52
	template <typename F,
		typename = std::enable_if_t<std::is_convertible_v<std::result_of_t<F(TArgs...)>,
			TReturn>>,
		typename = std::enable_if_t<std::is_invocable_r_v<TReturn, F, TArgs...>>,
		typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, functionRef>>
	>
	constexpr functionRef( F&& callable )
		:
		m_pAddr{ (void*)std::addressof( callable ) }
	{
		//static_assert(!std::is_member_function_pointer_v<F>,
		//	"This class doesn't accept member pointers yet..");
		if ( std::addressof( callable ) != nullptr )
		{
			m_pFn = [](void* po, TArgs... args) -> TReturn
			{
				return ( *reinterpret_cast<std::add_pointer_t<F>>( po ) )( std::forward<TArgs>( args )... );
			};
		}
		else
		{
			m_pAddr = nullptr;
			m_pFn = nullptr;
		}
	}

	// this is the destructor for functionRef not the underlying object
	~functionRef() noexcept
	{
		reset();
	}

	functionRef( const functionRef& rhs )
	{
		void* temp_addr{ rhs.m_pAddr };
		CallbackPointerType temp_pfn{ rhs.m_pFn };
		std::swap( m_pAddr, temp_addr );
		std::swap( m_pFn, temp_pfn );
	}

	functionRef& operator=( const functionRef& rhs )
	{
		if ( this != &rhs )
		{
			void* temp_addr{ rhs.m_pAddr };
			CallbackPointerType temp_pfn{ rhs.m_pFn };
			std::swap( m_pAddr, temp_addr );
			std::swap( m_pFn, temp_pfn );
		}
		return *this;
	}

	functionRef( functionRef&& rhs ) noexcept
		:
		m_pAddr{ std::move( rhs.m_pAddr ) },
		m_pFn{ std::move( rhs.m_pFn ) }
	{}
	functionRef &operator=( functionRef&& rhs ) noexcept
	{
		if ( this != &rhs )
		{
			std::swap( m_pAddr, rhs.m_pAddr );
			std::swap( m_pFn, rhs.m_pFn );
		}
		return *this;
	}

	void swap( functionRef& rhs ) noexcept
	{
		std::swap( m_pAddr, rhs.m_pAddr );
		std::swap( m_pFn, rhs.m_pFn );
	}
	
	//===================================================
	//	\function	setTarget
	//	\brief  use setTarget to set a member pointer as a target
	//	\date	2019/11/20 20:55
	template <typename T, typename FType, 
		typename = std::enable_if_t<std::is_member_function_pointer_v<FType>>
	>
	void setTarget( T& obj,
		FType f )
	{
		if ( std::addressof( obj ) != nullptr )
		{
			m_pAddr = reinterpret_cast<void*>( std::addressof( obj ) );
			m_pFn = reinterpret_cast<TReturn(*)( void*, TArgs... )>( (void*&)f );
		}
		else
		{
			m_pAddr = nullptr;
			m_pFn = nullptr;
		}
	}

	// Delete r-value cappables. Not going to maange temporary callables here
	template <typename T, typename FType>
	void setTarget( T&& ) = delete;
	
	bool reset() noexcept
	{
		if ( m_pAddr || m_pFn )
		{
			m_pAddr = nullptr;
			m_pFn = nullptr;
			return true;
		}
		return false;
	}

	inline TReturn operator()( TArgs... args ) const
	{
		return m_pFn( m_pAddr, std::forward<TArgs>( args )... );
	}

	operator bool() const noexcept
	{
		return !( m_pAddr == nullptr || m_pFn == nullptr );
	}

	[[nodiscard]]
	inline constexpr bool operator==( const functionRef& rhs ) const noexcept
	{
		return this->m_pAddr == rhs.m_pAddr;
	}
	
	[[nodiscard]]
	inline constexpr bool operator!=( const functionRef& rhs ) const noexcept
	{
		return this->m_pAddr != rhs.m_pAddr;
	}
	
	[[nodiscard]]
	inline constexpr bool operator==( const functionRef* rhs ) const noexcept
	{
		return this->m_pAddr == rhs->m_pAddr;
	}

	[[nodiscard]]
	inline constexpr bool operator!=( const functionRef* rhs ) const noexcept
	{
		return this->m_pAddr != rhs->m_pAddr;
	}
};
